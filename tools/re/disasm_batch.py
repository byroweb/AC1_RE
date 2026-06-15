#!/usr/bin/env python3
"""disasm_batch.py - batch objdump + m2c decompile of overlay byte ranges.

A thin batch wrapper over the existing single-shot tools/ghidra/overlay2c.sh
pipeline (objdump -> objdump2m2c.py -> m2c).  Give it a list of (addr,len,name)
targets and it writes, for each:
    scratch/re/disasm/<name>.s   raw m2c-style asm (always)
    scratch/re/disasm/<name>.c   m2c decompiler output (best effort)

m2c chokes on hand-written binary-search dispatch / jump tables; when it fails
the .s is still written and the .c carries the error, so a partial batch never
aborts the whole run.

Targets (repeatable / combinable):
    --fn ADDR:LEN[:NAME]      e.g. --fn 0x8008b380:0xde0:vm_tick
    --json FILE               [{"addr":..,"len":..,"name":..}, ...]
                              (addr/len may be hex strings or ints)

Usage:
  tools/re/disasm_batch.py --fn 0x8008b380:0xde0:vm_tick --fn 0x8008c178:0x180:vm_load_threads
  tools/re/disasm_batch.py --json scratch/re/vm_funcs.json
  tools/re/disasm_batch.py --bin OTHER.bin --base 0x80050000 --fn 0x...:0x...:name
"""
import argparse
import json
import os
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OBJDUMP2M2C = os.path.join(REPO, "tools", "ghidra", "objdump2m2c.py")
OUT_DIR = os.path.join(REPO, "scratch", "re", "disasm")

DEFAULT_BIN = os.path.join(REPO, "overlays",
                           "mission_overlay_FDAT202_80050000.bin")
DEFAULT_BASE = 0x80050000

OBJDUMP = "mipsel-linux-gnu-objdump"
M2C_DIR = "/home/byron/Applications/m2c"
M2C_PY = os.path.join(M2C_DIR, ".venv", "bin", "python")
M2C = os.path.join(M2C_DIR, "m2c.py")


def _int(v):
    if isinstance(v, int):
        return v
    return int(v, 0)


def parse_fn(spec):
    """ADDR:LEN[:NAME] -> (addr, length, name)."""
    parts = spec.split(":")
    if len(parts) < 2:
        raise ValueError(f"--fn needs ADDR:LEN[:NAME], got {spec!r}")
    addr = _int(parts[0])
    length = _int(parts[1])
    name = parts[2] if len(parts) > 2 else f"fn_{addr:x}"
    return addr, length, name


def load_targets(args):
    targets = []
    for spec in args.fn or []:
        targets.append(parse_fn(spec))
    if args.json:
        with open(args.json) as f:
            for d in json.load(f):
                targets.append((_int(d["addr"]), _int(d["len"]),
                                d.get("name", f"fn_{_int(d['addr']):x}")))
    return targets


def disasm_one(binpath, base, addr, length, name):
    """objdump+m2c one target. Returns (ok_asm, ok_c, note)."""
    off = addr - base
    if off < 0 or off + length > os.path.getsize(binpath):
        return (False, False, f"range outside {os.path.basename(binpath)}")

    with open(binpath, "rb") as f:
        f.seek(off)
        slice_bytes = f.read(length)

    s_path = os.path.join(OUT_DIR, f"{name}.s")
    c_path = os.path.join(OUT_DIR, f"{name}.c")

    # 1. objdump the slice as little-endian MIPS-I at its runtime VMA.
    #    objdump needs a real (seekable) file for `-b binary`, so use a temp.
    with tempfile.NamedTemporaryFile(suffix=".bin") as tf:
        tf.write(slice_bytes)
        tf.flush()
        dump = subprocess.run(
            [OBJDUMP, "-D", "-b", "binary", "-m", "mips:isa32", "-EL",
             f"--adjust-vma={addr}", tf.name],
            capture_output=True)
    if dump.returncode != 0:
        return (False, False,
                f"objdump failed: {dump.stderr.decode(errors='replace')[:120]}")

    # 2. objdump text -> m2c-ready GAS asm (always written).
    conv = subprocess.run(
        [sys.executable, OBJDUMP2M2C, hex(addr), name],
        input=dump.stdout, capture_output=True)
    with open(s_path, "wb") as f:
        f.write(conv.stdout)
    if conv.returncode != 0:
        return (True, False,
                f".s written; objdump2m2c err: "
                f"{conv.stderr.decode(errors='replace')[:120]}")

    # 3. m2c decompile (best effort; jump tables make this fail).
    m2c = subprocess.run(
        [M2C_PY, M2C, "-t", "mips-ido-c", s_path],
        capture_output=True)
    if m2c.returncode == 0 and m2c.stdout.strip():
        with open(c_path, "wb") as f:
            f.write(m2c.stdout)
        return (True, True, "ok")
    # m2c failed: leave a .c stub recording why, keep the .s.
    err = (m2c.stderr or b"").decode(errors="replace")[:400]
    with open(c_path, "w") as f:
        f.write(f"/* m2c failed for {name} @0x{addr:x} (len 0x{length:x}).\n"
                f"   Likely a jump table / hand dispatch; use {name}.s.\n"
                f"   stderr:\n{err}\n*/\n")
    return (True, False, "m2c failed (jump table?); .s only")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", default=DEFAULT_BIN, help="overlay/binary to slice")
    ap.add_argument("--base", default=hex(DEFAULT_BASE),
                    help="VA the binary maps at (default 0x80050000)")
    ap.add_argument("--fn", action="append",
                    help="target ADDR:LEN[:NAME] (repeatable)")
    ap.add_argument("--json", help="JSON list of {addr,len,name} targets")
    args = ap.parse_args()

    base = _int(args.base)
    targets = load_targets(args)
    if not targets:
        ap.error("give at least one --fn ADDR:LEN[:NAME] or --json FILE")

    os.makedirs(OUT_DIR, exist_ok=True)
    n_asm = n_c = 0
    for addr, length, name in targets:
        ok_asm, ok_c, note = disasm_one(args.bin, base, addr, length, name)
        n_asm += int(ok_asm)
        n_c += int(ok_c)
        status = "OK " if ok_c else ("ASM" if ok_asm else "ERR")
        print(f"[{status}] {name:24s} 0x{addr:08x} len 0x{length:<5x} {note}")
    print(f"\nwrote {n_asm} .s and {n_c} .c into {os.path.relpath(OUT_DIR, REPO)}/")


if __name__ == "__main__":
    main()
