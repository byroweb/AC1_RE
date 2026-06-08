#!/usr/bin/env python3
"""
pa_parse.py — structural parser for AC1 PA##.T stage/map packs.

PA##.T is a standard count-first ".T" container (see tools/extract_t.py). Its
entries are NOT Sony TMD/TIM — AC1 uses a *custom* geometry/map format. This tool
dumps the confirmed structure so the format can be cracked incrementally:

  entry 0      : map master header (size, signature 0x03072d39, pointer table,
                 object-index list) — layout TBD
  entry 1      : 2048 B offset directory (uint16 table) — layout TBD
  entry 2..N   : size-prefixed geometry blocks. Each begins:
                   u32[0] = block size in bytes (== entry length)   CONFIRMED
                   u32[2] = sub-object count (?)                     hypothesis
                   then a table of (offset, count) sub-sections.
                 Sub-sections seen: an int16 vertex array (stride 8: x,y,z,flag)
                 and fixed-8-byte primitive records (custom, not TMD packets).

Usage:
  python3 tools/pa_parse.py GG/P0/PA00.T                 # summary of all entries
  python3 tools/pa_parse.py GG/P0/PA00.T --entry 2 -v    # dump one block's header
  python3 tools/pa_parse.py GG/P0/PA00.T --entry 2 --verts 0x1218 122  # decode verts
"""
import json, struct, sys, argparse, collections, os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")
RAW, OFF, DATA = 2352, 24, 2048


def load_container(path):
    fm = json.load(open(FILEMAP))
    rec = next((r for r in fm["files"] if r["path"] == path
                or r["path"].endswith("/" + path)), None)
    if not rec:
        sys.exit(f"not in filemap: {path}")
    blob = bytearray()
    with open(fm["source_bin"], "rb") as f:
        for s in range(rec["sector_first"], rec["sector_last"] + 1):
            f.seek(s * RAW + OFF)
            blob += f.read(DATA)
    blob = bytes(blob)
    n = struct.unpack_from("<H", blob, 0)[0]
    offs = list(struct.unpack_from(f"<{n+1}H", blob, 2))
    entries = []
    for i in range(n):
        a, b = offs[i] * 2048, offs[i + 1] * 2048
        entries.append(blob[a:b])
    return entries


def summary(entries):
    print(f"{len(entries)} entries ({sum(1 for e in entries if e)} non-empty)")
    sizematch = 0
    for i, e in enumerate(entries):
        if not e:
            continue
        prefix = struct.unpack_from("<I", e, 0)[0] if len(e) >= 4 else 0
        kind = "geom-block" if prefix == len(e) else ("hdr/table" if i < 2 else "?")
        if prefix == len(e):
            sizematch += 1
        if i < 4 or (prefix != len(e) and len(e) > 4):
            tag = struct.unpack_from("<I", e, 4)[0] if len(e) >= 8 else 0
            print(f"  entry {i:3d}  len={len(e):6d}  u32[0]=0x{prefix:08x}  "
                  f"u32[1]=0x{tag:08x}  {kind}")
    print(f"  size-prefixed geometry blocks: {sizematch}")


def dump_block_header(e, words=24):
    print(f"block len={len(e)}  first {words} u32:")
    for i in range(0, words):
        if (i + 1) * 4 > len(e):
            break
        v = struct.unpack_from("<I", e, i * 4)[0]
        a, b = struct.unpack_from("<2H", e, i * 4)
        note = ""
        if 0 < v < len(e):
            note = f"  -> in-block offset (0x{v:x})"
        print(f"  u32[{i:2d}] @0x{i*4:03x} = 0x{v:08x} ({v:>8})   u16=({a:5d},{b:5d}){note}")


def decode_verts(e, off, cnt):
    print(f"vertices @0x{off:x} x{cnt} (int16 x,y,z,flag stride 8):")
    xs = ys = zs = None
    pts = []
    for i in range(cnt):
        if off + i * 8 + 8 > len(e):
            break
        x, y, z, w = struct.unpack_from("<4h", e, off + i * 8)
        pts.append((x, y, z, w))
    for p in pts[:10]:
        print("   ", p)
    if pts:
        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]; zs = [p[2] for p in pts]
        print(f"  X {min(xs)}..{max(xs)}  Y {min(ys)}..{max(ys)}  Z {min(zs)}..{max(zs)}")
        print(f"  end offset = 0x{off + cnt*8:x}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--entry", type=int)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--verts", nargs=2, metavar=("OFF", "CNT"),
                    help="decode int16 vertex array at OFF (hex ok) for CNT verts")
    args = ap.parse_args()

    entries = load_container(args.path)
    if args.entry is None:
        summary(entries)
        return
    e = entries[args.entry]
    print(f"=== {args.path} entry {args.entry} (len {len(e)}) ===")
    if args.verts:
        decode_verts(e, int(args.verts[0], 0), int(args.verts[1], 0))
    else:
        dump_block_header(e, 24 if args.verbose else 12)


if __name__ == "__main__":
    main()
