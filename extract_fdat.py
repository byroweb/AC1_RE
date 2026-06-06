#!/usr/bin/env python3
"""
extract_fdat.py — extract FDAT.T from AC1 BIN as a flat 2048-byte/sector file.
Usage: python3 extract_fdat.py [input.bin] [output.T]
"""
import sys, os, struct

BIN_DEFAULT = "/home/byron/Desktop/Armored_Core_Hacks/AC_1_USA_test/Armored Core (v1.1).bin"
OUT_DEFAULT = "/home/byron/Desktop/AC_1_USA_RE/fdat_extracted.T"

SECTOR_RAW  = 2352
DATA_OFFSET = 24        # Mode 2 Form 1: 12 sync + 4 header + 8 subheader
DATA_SIZE   = 2048

FDAT_FIRST  = 72189
FDAT_COUNT  = 13142     # sectors 72189–85330 inclusive (85330-72189+1); 13142×2048=26,914,816

def main():
    bin_path = sys.argv[1] if len(sys.argv) > 1 else BIN_DEFAULT
    out_path = sys.argv[2] if len(sys.argv) > 2 else OUT_DEFAULT

    expected_bytes = FDAT_COUNT * DATA_SIZE   # 26,912,768 — close to 26,914,816
    # jPSXdec reports 26914816; FDAT_COUNT = ceil(26914816/2048) = 13141
    # last sector may be partially filled — we write full 2048 per sector

    print(f"Input : {bin_path}")
    print(f"Output: {out_path}")
    print(f"Sectors: {FDAT_FIRST} – {FDAT_FIRST + FDAT_COUNT - 1} inclusive ({FDAT_COUNT} sectors)")
    print(f"Expected output size: {FDAT_COUNT * DATA_SIZE:,} bytes")

    with open(bin_path, "rb") as f_in, open(out_path, "wb") as f_out:
        for i, s in enumerate(range(FDAT_FIRST, FDAT_FIRST + FDAT_COUNT)):
            f_in.seek(s * SECTOR_RAW + DATA_OFFSET)
            data = f_in.read(DATA_SIZE)
            if len(data) != DATA_SIZE:
                raise RuntimeError(f"Short read at sector {s} (got {len(data)} bytes)")
            f_out.write(data)
            if (i + 1) % 1000 == 0:
                print(f"  {i+1}/{FDAT_COUNT} sectors…")

    actual = os.path.getsize(out_path)
    print(f"Done. Written {actual:,} bytes to {out_path}")

    # Quick sanity: check TOC magic (first ushort = entry count = 205)
    with open(out_path, "rb") as f:
        entry_count = struct.unpack_from("<H", f.read(2))[0]
    print(f"FDAT TOC entry count: {entry_count}  {'✓' if entry_count == 205 else '✗ UNEXPECTED'}")

if __name__ == "__main__":
    main()
