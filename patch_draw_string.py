#!/usr/bin/env python3
"""
patch_draw_string.py — patch compiled draw_string.bin into fdat_extracted.T

draw_string region in FDAT entry 201:
  flat offset : 25,658,364  (entry201 base 25,548,800 + entry offset 109,564)
  region size : 1,056 bytes  (draw_string start to draw_kanji start)
  draw_kanji  : starts at flat offset 25,659,420 — NOT patched here

Usage: python3 patch_draw_string.py [draw_string_fn.bin] [fdat_extracted.T]
"""
import sys, os

BIN_DEFAULT  = "draw_string_fn.bin"
FDAT_DEFAULT = "fdat_extracted.T"

PATCH_OFFSET = 25_658_364   # flat offset of draw_string in fdat_extracted.T
REGION_SIZE  = 1_056         # bytes from draw_string start to draw_kanji start

# FDAT entry 201 carries a trailing checksum word verified by the overlay loader
# (FUN_80015b24): seed 0x12345678 + sum of all preceding 32-bit words must equal
# the last word. Any edit to entry 201 MUST update it or the load loops forever.
E201_FLAT  = 25_548_800
E201_SIZE  = 542_720
E201_SEED  = 0x12345678

def fix_overlay_checksum(fdat_path):
    import struct
    nwords = E201_SIZE // 4
    with open(fdat_path, "r+b") as f:
        f.seek(E201_FLAT)
        words = struct.unpack(f"<{nwords}I", f.read(E201_SIZE))
        s = E201_SEED
        for w in words[:nwords - 1]:
            s = (s + w) & 0xFFFFFFFF
        f.seek(E201_FLAT + E201_SIZE - 4)
        f.write(struct.pack("<I", s))
    print(f"Entry 201 checksum updated -> 0x{s:08X}")

def main():
    code_path = sys.argv[1] if len(sys.argv) > 1 else BIN_DEFAULT
    fdat_path = sys.argv[2] if len(sys.argv) > 2 else FDAT_DEFAULT

    code = open(code_path, "rb").read()
    print(f"Code binary : {code_path} ({len(code)} bytes)")
    print(f"FDAT file   : {fdat_path} ({os.path.getsize(fdat_path):,} bytes)")
    print(f"Patch offset: {PATCH_OFFSET:,}  region: {REGION_SIZE} bytes")

    if len(code) > REGION_SIZE:
        raise RuntimeError(f"Binary {len(code)} bytes > region {REGION_SIZE} bytes")

    pad = REGION_SIZE - len(code)
    print(f"NOP padding : {pad} bytes ({pad // 4} words)")

    with open(fdat_path, "r+b") as f:
        f.seek(PATCH_OFFSET)
        f.write(code)
        f.write(b"\x00\x00\x00\x00" * (pad // 4))

    print("Patch written. Verifying offset...")
    with open(fdat_path, "rb") as f:
        f.seek(PATCH_OFFSET)
        verify = f.read(4)
    import struct
    first_word = struct.unpack_from("<I", verify)[0]
    expected   = struct.unpack_from("<I", code)[0]
    ok = "✓" if first_word == expected else "✗ MISMATCH"
    print(f"First word at patch offset: 0x{first_word:08X} {ok}")

    # MUST run after writing code: recompute entry 201's trailing checksum
    fix_overlay_checksum(fdat_path)

if __name__ == "__main__":
    main()
