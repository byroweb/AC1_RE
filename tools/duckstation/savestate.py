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

The save-state LIBRARY (archive slots under stable IDs, manifest in
disc_map/states/STATES.tsv) is the sibling tool tools/duckstation/states.py; it
imports this module for the header/RAM readers.

Container format (DuckStation "DUCCS")
--------------------------------------
Matches SAVE_STATE_HEADER in the emulator's src/core/save_state_version.h
(checked 2026-10-17 against the sadnescity `mcp` fork). Little-endian, packed to 4:
  +0x00  u32  magic 'DUCC' (0x43435544)
  +0x04  u32  version                 (the 'S\\0\\0\\0' seen in the 2026-06 backups = 83)
  +0x08  char[128] title              ("Armored Core (Reprint)")
  +0x88  char[32]  serial             ("SLUS-01323")
  +0xa8  u32  media_path_length       (NOT the state version — the earlier "82" read
                                       here was the length of the disc path string)
  +0xac  u32  offset_to_media_path
  +0xb0  u32  media_subimage_index
  +0xb4  u32  screenshot_compression_type   (0 none, 1 deflate, 2 zstd, 3 xz; >= v69)
  +0xb8  u32  screenshot_width
  +0xbc  u32  screenshot_height
  +0xc0  u32  screenshot_compressed_size
  +0xc4  u32  offset_to_screenshot          (pixels are RGBA8, pitch*height bytes)
  +0xc8  u32  data_compression_type         (same enum)
  +0xcc  u32  data_compressed_size
  +0xd0  u32  data_uncompressed_size        (3,819,074 for AC1)
  +0xd4  u32  offset_to_data
The main state is ONE compressed frame at +0xd4; decompressed it is the concatenated
component serialization (System, CPU, Bus RAM, GPU/VRAM, SPU RAM, ...). Each
component is preceded by a StateWrapper marker (u32 length + ASCII name, e.g.
"\\x06\\0\\0\\0System", "\\x03\\0\\0\\0CPU"), but fields inside a component carry no
framing, so RAM/VRAM are located by CONTENT, not by a fixed offset:

  RAM base: the game EXE (SLUS_013.23) loads at its PSX-EXE t_addr (0x80011e6c for
  AC1 v1.1). The first bytes of the EXE payload therefore appear in the decompressed
  blob at  ram_base + (t_addr - 0x80000000).  Find that byte run -> ram_base.

Once ram_base is known, PS1 address A (0x800xxxxx) is blob[ram_base + (A-0x80000000)].

Decompression backends (first available wins): the `zstandard` Python package,
then the `zstd` CLI on PATH. Deflate/xz frames use the stdlib. zstd_backend()
reports which one is active; states.py degrades to "fields unknown" without one.
"""
from __future__ import annotations
import struct, subprocess, sys, os, shutil, zlib, lzma

RAM_SIZE = 2 * 1024 * 1024          # PS1 main RAM
VRAM_SIZE = 1024 * 512 * 2          # PS1 GPU VRAM (1024x512 16bpp)
PS1_RAM_BASE = 0x80000000
MAGIC = 0x43435544                  # 'DUCC'

COMPRESSION_NONE, COMPRESSION_DEFLATE, COMPRESSION_ZSTD, COMPRESSION_XZ = 0, 1, 2, 3
COMPRESSION_NAMES = {0: "none", 1: "deflate", 2: "zstd", 3: "xz"}

HEADER_FIELDS = (
    ("magic", 0x00), ("version", 0x04),
    ("media_path_length", 0xa8), ("offset_to_media_path", 0xac),
    ("media_subimage_index", 0xb0),
    ("screenshot_compression_type", 0xb4), ("screenshot_width", 0xb8),
    ("screenshot_height", 0xbc), ("screenshot_compressed_size", 0xc0),
    ("offset_to_screenshot", 0xc4),
    ("data_compression_type", 0xc8), ("data_compressed_size", 0xcc),
    ("data_uncompressed_size", 0xd0), ("offset_to_data", 0xd4),
)
HEADER_SIZE = 0xd8


def zstd_backend():
    """'zstandard' (python package), 'zstd' (CLI), or None if neither is available."""
    try:
        import zstandard  # noqa: F401
        return "zstandard"
    except ImportError:
        pass
    if shutil.which("zstd"):
        return "zstd"
    return None


def _zstd_decompress(frame: bytes, expected=None) -> bytes:
    be = zstd_backend()
    if be == "zstandard":
        import zstandard
        d = zstandard.ZstdDecompressor()
        if expected:
            return d.decompress(frame, max_output_size=expected)
        return d.decompress(frame)
    if be == "zstd":
        p = subprocess.run(["zstd", "-dqc"], input=frame, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
        if p.returncode != 0:
            raise RuntimeError("zstd -d failed: " + p.stderr.decode("utf-8", "replace"))
        return p.stdout
    raise RuntimeError("no zstd decoder: `pip install zstandard` or put the zstd CLI on PATH")


def decompress(frame: bytes, ctype: int, expected=None) -> bytes:
    """Decompress one DuckStation block according to its CompressionType enum."""
    if ctype == COMPRESSION_NONE:
        return frame
    if ctype == COMPRESSION_DEFLATE:
        return zlib.decompress(frame)
    if ctype == COMPRESSION_ZSTD:
        return _zstd_decompress(frame, expected)
    if ctype == COMPRESSION_XZ:
        return lzma.decompress(frame)
    raise ValueError(f"unknown compression type {ctype}")


def _cstr(b: bytes) -> str:
    return b.split(b"\0", 1)[0].decode("utf-8", "replace")


def read_header(path_or_bytes) -> dict:
    """Parse the DUCCS header into a dict (ints + decoded title/serial/media_path)."""
    if isinstance(path_or_bytes, (bytes, bytearray)):
        d = bytes(path_or_bytes)
        getrange = lambda off, n: d[off:off + n]
    else:
        f = open(path_or_bytes, "rb")
        d = f.read(HEADER_SIZE)

        def getrange(off, n):
            f.seek(off)
            return f.read(n)
    if len(d) < HEADER_SIZE:
        raise ValueError(f"{path_or_bytes if not isinstance(path_or_bytes, bytes) else 'buffer'}: "
                         f"too short for a DUCCS header")
    h = {k: struct.unpack_from("<I", d, off)[0] for k, off in HEADER_FIELDS}
    if h["magic"] != MAGIC:
        raise ValueError(f"not a DuckStation state (magic {d[:4]!r})")
    h["title"] = _cstr(d[0x08:0x88])
    h["serial"] = _cstr(d[0x88:0xa8])
    h["media_path"] = getrange(h["offset_to_media_path"],
                               h["media_path_length"]).decode("utf-8", "replace")
    return h


def read_main_state(path: str) -> bytes:
    """Decompress and return the main-state blob (concatenated components)."""
    d = open(path, "rb").read()
    h = read_header(d)
    off, clen, ulen = h["offset_to_data"], h["data_compressed_size"], h["data_uncompressed_size"]
    blob = decompress(d[off:off + clen], h["data_compression_type"], ulen)
    if len(blob) != ulen:
        raise RuntimeError(f"decompressed {len(blob)} != header ulen {ulen}")
    return blob


def read_screenshot(path: str):
    """Return (width, height, rgba8_bytes) for the embedded thumbnail, or None."""
    d = open(path, "rb").read()
    h = read_header(d)
    w, hh = h["screenshot_width"], h["screenshot_height"]
    if not w or not hh or not h["screenshot_compressed_size"]:
        return None
    off, clen = h["offset_to_screenshot"], h["screenshot_compressed_size"]
    ctype = h["screenshot_compression_type"] if h["version"] >= 69 else COMPRESSION_NONE
    pix = decompress(d[off:off + clen], ctype, w * hh * 4 * 2)
    if len(pix) < w * hh * 4 or len(pix) % hh:
        raise RuntimeError(f"screenshot payload {len(pix)} bytes does not fit {w}x{hh} RGBA8")
    pitch = len(pix) // hh
    if pitch != w * 4:                      # pitch may be padded; drop the padding
        pix = b"".join(pix[y * pitch: y * pitch + w * 4] for y in range(hh))
    return w, hh, pix


def frame_numbers(blob: bytes):
    """(frame_number, internal_frame_number) from the System component, or None.

    System::DoState writes: marker "System", region (u8 from v74, u32 before),
    taints u32 (from v75), frame_number u32, internal_frame_number u32, BIOS hash
    16 bytes (from v58), then the marker "CPU". Anchoring on the CPU marker makes
    the read independent of the region/taint width. Layout checked against the
    emulator source, not yet against a real state — see docs/SAVESTATE_FORMAT.md.
    """
    if not blob.startswith(b"\x06\x00\x00\x00System"):
        return None
    j = blob.find(b"\x03\x00\x00\x00CPU", 0, 96)
    if j < 24:
        return None
    return struct.unpack_from("<II", blob, j - 24)


def find_ram_base(blob: bytes, anchor: bytes, anchor_addr: int) -> int:
    """anchor = >=32 bytes known to sit at PS1 address anchor_addr in RAM."""
    j = blob.find(anchor)
    if j < 0:
        raise RuntimeError("RAM anchor not found in state blob")
    return j - (anchor_addr - PS1_RAM_BASE)


def exe_anchor_from_disc(disc_bin: str, n: int = 256):
    """Return (anchor_bytes, t_addr) by reading the PSX-EXE off a MODE2/2352 disc.

    PS-X EXE header: +0x10 pc0 (entry point), +0x14 gp0, +0x18 t_addr (RAM load
    address of the payload), +0x1c t_size. Until 2026-10-18 this read +0x10, i.e.
    the entry PC (0x80011e6c) instead of the load address (0x80011000) — the
    resulting ram_base was 0xe6c too low and every RAM read landed on the wrong
    bytes (docs/SAVESTATE_FORMAT.md "Correction").
    """
    RAW, OFF, DS = 2352, 24, 2048
    with open(disc_bin, "rb") as f:
        data = f.read(64 * 1024 * 1024)          # the EXE sits in the first MBs
    magic = data.find(b"PS-X EXE")
    if magic < 0:
        raise RuntimeError("PS-X EXE not found on disc")
    s0 = (magic - OFF) // RAW
    sect = lambda s: data[s * RAW + OFF: s * RAW + OFF + DS]
    head = b"".join(sect(s0 + k) for k in range(90))
    t_addr = struct.unpack_from("<I", head, 0x18)[0]
    fsize = struct.unpack_from("<I", head, 0x1c)[0]
    payload = head[0x800:0x800 + fsize]
    return payload[:n], t_addr


VRAM_MARKER = b"\x08\x00\x00\x00GPU-VRAM"   # StateWrapper section marker


def vram_offset(blob: bytes):
    """Offset of VRAM in a decompressed state: right after the "\\x08\\0\\0\\0GPU-VRAM"
    marker inside the GPU component (followed by "GPUTextureCache"). The GPU
    registers before it vary in length (0x2bb vs 0x303 bytes after the "GPU" marker
    in the v87 states), so a fixed offset is wrong. None if there is no marker."""
    i = blob.find(VRAM_MARKER)
    if i < 0 or i + len(VRAM_MARKER) + VRAM_SIZE > len(blob):
        return None
    return i + len(VRAM_MARKER)


def bus_marker_offset(blob: bytes) -> int:
    """Offset of the "Bus" component marker (RAM follows a few bytes later), or -1."""
    return blob.find(b"\x03\x00\x00\x00Bus", 0, 0x10000)


class SaveState:
    def __init__(self, path: str, disc_bin: str = None,
                 anchor: bytes = None, anchor_addr: int = None):
        self.path = path
        self.header = read_header(path)
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
        """The 1 MB VRAM block (1024x512 16bpp), found by its "GPU-VRAM" marker
        (vram_offset). Falls back to the old guess just past RAM, which is wrong
        for v87 states, only when the marker is missing."""
        o = vram_offset(self.blob)
        if o is None:
            o = self.ram_base + RAM_SIZE
        return self.blob[o: o + VRAM_SIZE]

    def vram_at(self, off: int) -> bytes:
        return self.blob[off: off + VRAM_SIZE]

    # --- mission identity ----------------------------------------------------
    def mission_byte(self) -> int:
        return self.u8(0x800411F8)

    def stage_byte(self) -> int:
        return self.u8(0x8004121B)

    def in_mission(self) -> bool:
        """True if a mission is live (objective object bound + stage set)."""
        return self.u32(0x8019F51C) != 0 or self.stage_byte() != 0


def _main(argv):
    import argparse
    ap = argparse.ArgumentParser(description="DuckStation save-state RAM/VRAM extractor")
    ap.add_argument("state", help="path to SLUS-01323_*.sav")
    ap.add_argument("--disc", default=os.environ.get("AC1_BIN"),
                    help="disc .bin for the EXE RAM anchor (default: $AC1_BIN)")
    ap.add_argument("--header", action="store_true", help="print the DUCCS header and exit")
    ap.add_argument("--dump-ram", help="write 2 MB RAM to this file")
    ap.add_argument("--dump-vram", help="write 1 MB VRAM to this file")
    ap.add_argument("--read", help="PS1 hex address to read (u32), e.g. 0x8019F51C")
    a = ap.parse_args(argv)
    if a.header:
        for k, v in read_header(a.state).items():
            print(f"{k:28s} {v:#x}" if isinstance(v, int) else f"{k:28s} {v}")
        return
    if not a.disc:
        ap.error("--disc (or $AC1_BIN) is required unless --header")
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
