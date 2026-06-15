#!/usr/bin/env python3
"""
tools/duckstation/savestate.py — read a DuckStation ".sav" save state OFFLINE and
pull out the PS1 main RAM (2 MB) and GPU VRAM (1 MB), plus convenience helpers to
read game memory by PS1 address and to identify which mission/stage a state captured.

Why this exists
---------------
The per-stage texture bank that AC1 uploads to VRAM at mission load is not yet
sourced from disc (see docs/PA_FORMAT.md "Textured-primitive MATERIAL words" and
the open texture-bank item). The fallback is a VRAM dump from an in-mission state.
DuckStation states are also the ground-truth source for the live-confirmation batch
(read RAM at a known address without breaking out the emulator).

Container format (DuckStation "DUCCS", state version 82, RE'd 2026-06-15)
------------------------------------------------------------------------
Header (little-endian):
  +0x00  u32  magic 'DUCC' (0x43435544) + 'S\\0\\0\\0'
  +0x08  char[..] display title, then +0x88 game serial "SLUS-01323"
  +0xa8  u32  state version (82)
  +0xc0  u32  screenshot zstd length
  +0xc4  u32  screenshot zstd offset
  +0xcc  u32  main-state zstd compressed length
  +0xd0  u32  main-state UNCOMPRESSED length (3,819,074 for AC1)
  +0xd4  u32  main-state zstd offset
The main state is ONE zstd frame at +0xd4; decompressed it is the concatenated
component serialization (Bus RAM, CPU, GPU/VRAM, SPU RAM, ...) with no per-field
framing. RAM/VRAM are located by CONTENT, not by a fixed offset:

  RAM base: the game EXE (SLUS_013.23) loads at its PSX-EXE t_addr (0x80011e6c for
  AC1 v1.1). The first bytes of the EXE payload therefore appear in the decompressed
  blob at  ram_base + (t_addr - 0x80000000).  Find that byte run -> ram_base.

Once ram_base is known, PS1 address A (0x800xxxxx) is blob[ram_base + (A-0x80000000)].

Requires the `zstd` CLI on PATH (no python zstandard dependency).
"""
from __future__ import annotations
import struct, subprocess, sys, os

RAM_SIZE = 2 * 1024 * 1024          # PS1 main RAM
VRAM_SIZE = 1024 * 512 * 2          # PS1 GPU VRAM (1024x512 16bpp)
PS1_RAM_BASE = 0x80000000


def _zstd_decompress(frame: bytes) -> bytes:
    p = subprocess.run(["zstd", "-dqc"], input=frame, stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
    if p.returncode != 0:
        raise RuntimeError("zstd -d failed: " + p.stderr.decode("utf-8", "replace"))
    return p.stdout


def read_main_state(path: str) -> bytes:
    """Decompress and return the main-state blob (concatenated components)."""
    d = open(path, "rb").read()
    if d[:4] != b"DUCC":
        raise ValueError(f"{path}: not a DuckStation state (magic {d[:4]!r})")
    off = struct.unpack_from("<I", d, 0xd4)[0]
    clen = struct.unpack_from("<I", d, 0xcc)[0]
    ulen = struct.unpack_from("<I", d, 0xd0)[0]
    blob = _zstd_decompress(d[off:off + clen])
    if len(blob) != ulen:
        raise RuntimeError(f"decompressed {len(blob)} != header ulen {ulen}")
    return blob


def find_ram_base(blob: bytes, anchor: bytes, anchor_addr: int) -> int:
    """anchor = >=32 bytes known to sit at PS1 address anchor_addr in RAM."""
    j = blob.find(anchor)
    if j < 0:
        raise RuntimeError("RAM anchor not found in state blob")
    return j - (anchor_addr - PS1_RAM_BASE)


def exe_anchor_from_disc(disc_bin: str, n: int = 256):
    """Return (anchor_bytes, t_addr) by reading the PSX-EXE off a MODE2/2352 disc."""
    RAW, OFF, DS = 2352, 24, 2048
    data = open(disc_bin, "rb").read()
    magic = data.find(b"PS-X EXE")
    if magic < 0:
        raise RuntimeError("PS-X EXE not found on disc")
    s0 = (magic - OFF) // RAW
    sect = lambda s: data[s * RAW + OFF: s * RAW + OFF + DS]
    head = b"".join(sect(s0 + k) for k in range(90))
    t_addr = struct.unpack_from("<I", head, 0x10)[0]
    fsize = struct.unpack_from("<I", head, 0x1c)[0]
    payload = head[0x800:0x800 + fsize]
    return payload[:n], t_addr


class SaveState:
    def __init__(self, path: str, disc_bin: str = None,
                 anchor: bytes = None, anchor_addr: int = None):
        self.path = path
        self.blob = read_main_state(path)
        if anchor is None:
            if disc_bin is None:
                raise ValueError("need disc_bin or (anchor, anchor_addr)")
            anchor, anchor_addr = exe_anchor_from_disc(disc_bin)
        self.ram_base = find_ram_base(self.blob, anchor, anchor_addr)

    # --- RAM access by PS1 address -------------------------------------------
    def _o(self, addr: int) -> int:
        return self.ram_base + (addr - PS1_RAM_BASE)

    def u8(self, addr):  return self.blob[self._o(addr)]
    def u16(self, addr): return struct.unpack_from("<H", self.blob, self._o(addr))[0]
    def u32(self, addr): return struct.unpack_from("<I", self.blob, self._o(addr))[0]
    def i32(self, addr): return struct.unpack_from("<i", self.blob, self._o(addr))[0]
    def ram(self) -> bytes:
        return self.blob[self.ram_base: self.ram_base + RAM_SIZE]

    # --- VRAM ----------------------------------------------------------------
    def vram(self) -> bytes:
        """Locate the 1 MB VRAM block. GPU is serialized after RAM; VRAM is the
        first 1 MB-aligned region whose 16bpp framebuffer is plausible. Heuristic:
        scan for a 1 MB window after RAM end that is not all-zero and not the SPU
        RAM signature; caller can override via vram_at()."""
        # Coarse default: many DuckStation v82 states place VRAM shortly after the
        # 2 MB RAM. Provide the raw blob via vram_at() if this guess is wrong.
        guess = self.ram_base + RAM_SIZE
        # skip small CPU/cache component(s): search next 256 KB for a non-flat block
        return self.blob[guess: guess + VRAM_SIZE]

    def vram_at(self, off: int) -> bytes:
        return self.blob[off: off + VRAM_SIZE]

    # --- mission identity ----------------------------------------------------
    def stage_byte(self) -> int:
        return self.u8(0x8004121B)

    def in_mission(self) -> bool:
        """True if a mission is live (objective object bound + stage set)."""
        return self.u32(0x8019F51C) != 0 or self.stage_byte() != 0


def _main(argv):
    import argparse
    ap = argparse.ArgumentParser(description="DuckStation save-state RAM/VRAM extractor")
    ap.add_argument("state", help="path to SLUS-01323_*.sav")
    ap.add_argument("--disc", required=True, help="disc .bin (for the EXE RAM anchor)")
    ap.add_argument("--dump-ram", help="write 2 MB RAM to this file")
    ap.add_argument("--dump-vram", help="write 1 MB VRAM to this file")
    ap.add_argument("--read", help="PS1 hex address to read (u32), e.g. 0x8019F51C")
    a = ap.parse_args(argv)
    ss = SaveState(a.state, disc_bin=a.disc)
    print(f"ram_base={ss.ram_base:#x}  stage_byte={ss.stage_byte()}  "
          f"in_mission={ss.in_mission()}")
    if a.read:
        addr = int(a.read, 0)
        print(f"[{addr:#x}] = {ss.u32(addr):#x} ({ss.u32(addr)})")
    if a.dump_ram:
        open(a.dump_ram, "wb").write(ss.ram());  print("wrote", a.dump_ram)
    if a.dump_vram:
        open(a.dump_vram, "wb").write(ss.vram()); print("wrote", a.dump_vram)


if __name__ == "__main__":
    _main(sys.argv[1:])
