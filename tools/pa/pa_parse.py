#!/usr/bin/env python3
"""
pa_parse.py — structural parser for AC1 PA##.T stage/map packs.

PA##.T is a standard count-first ".T" container (see tools/extract/extract_t.py). Its
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
  python3 tools/pa/pa_parse.py GG/P0/PA00.T                 # summary of all entries
  python3 tools/pa/pa_parse.py GG/P0/PA00.T --entry 2 -v    # dump one block's header
  python3 tools/pa/pa_parse.py GG/P0/PA00.T --entry 2 --verts 0x1218 122  # decode verts
"""
import json, struct, sys, argparse, collections, os

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")
RAW, OFF, DATA = 2352, 24, 2048


def load_container(path):
    fm = json.load(open(FILEMAP))
    rec = next((r for r in fm["files"]
                if r.get("path") == path
                or (r.get("path") or "").endswith("/" + path)), None)
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


# --- Primitive record decode (RE'd from overlay emitter 0x8005A57C, relocation
#     walker 0x800574D8, jump table 0x8004B184). ---------------------------------
# Each record: byte[1] = record length in 32-bit words (record bytes = 4+words*4);
# byte[3] = primitive type, masked with 0xBC (so 0x02 toggles a variant and the
# 0x80 bit = textured). The renderer dispatches on (type & 0xFD) ⇒ {0x24,0x2c,
# 0x34,0x3c} flat/gouraud, {0xa4,0xac,...} textured.  Per type, certain record
# halfwords are VERTEX indices: in the *on-disc* file they are raw small indices,
# which the relocation pass (0x800574D8) multiplies (×8 for the stride-8 transformed
# vertex pool / ×16 for the colour-normal pool) into byte offsets. We decode the
# raw on-disc form here and report vertex indices for range-checking.
#
# Primitive record byte layout (CONFIRMED by disasm + byte-validation on PA00/PA20):
#   byte[0] = (#verts*2)+ (3..5)  small length tag        byte[1] = record length in WORDS
#   byte[3] = primitive type (the renderer masks it 0xBC; 0x80 bit = textured)
#   then the per-vertex *shading* block:
#     FLAT  (0x20/0x28): one RGB+code word  "RR GG BB cc" at +4
#     TEX   (0x24/0x2c/0x34/0x3c): UV0+clut, UV1+tpage, UV2 (+UV3) words at +4..
#   then N VERTEX INDICES (uint16 each) into this sub-object's vertex pool,
#   then an optional trailing per-face flag word (often 0).
# (vidx_off, nverts) per type — record-relative byte offset of the first index.
# Verified: PA00 e2 @0x1960 (flat quads), PA20 e3/e10/e13/e16 (textured tris+quads).
PRIM_VERTS = {
    0x20: (0x08, 3),   # flat tri        len 16
    0x28: (0x08, 4),   # flat quad       len 20
    0x24: (0x12, 3),   # textured tri    len 24
    0x2c: (0x14, 4),   # textured quad   len 32
    0x34: (0x12, 3),   # gouraud/tex tri len 28
    0x3c: (0x12, 4),   # textured quad   len 36
    # textured-variant high types share the same tail-index layout:
    0xa0: (0x0a, 3), 0xa8: (0x0a, 4),
    0xa4: (0x12, 3), 0xac: (0x16, 4),
    0xb4: (0x12, 3), 0xbc: (0x12, 4),
}


def decode_prims(e, off, cnt):
    print(f"primitive stream @0x{off:x} (walk by byte[1]=word-length, "
          f"type=byte[3]&0xbc):")
    o = off
    seen = 0
    types = collections.Counter()
    maxidx = -1
    while seen < cnt and o + 4 <= len(e):
        b0, b1, b2, b3 = e[o], e[o + 1], e[o + 2], e[o + 3]
        words = b1
        reclen = 4 + words * 4
        if reclen < 4 or o + reclen > len(e):
            print(f"  @0x{o:04x}  STOP (reclen {reclen} OOB)")
            break
        typ = b3 & 0xbc
        info = PRIM_VERTS.get(typ)
        vstr = ""
        if info:
            voff, nv = info
            vidx = []
            for k in range(nv):
                p = o + voff + 2 * k
                if p + 2 <= len(e):
                    vidx.append(struct.unpack_from("<H", e, p)[0])
            vstr = f"  verts={vidx}"
            maxidx = max([maxidx] + vidx)
        types[typ] += 1
        raw = " ".join(f"{x:02x}" for x in e[o:o + min(reclen, 24)])
        if seen < 16:
            print(f"  @0x{o:04x} type=0x{typ:02x} words={words} len={reclen}"
                  f"{vstr}   [{raw}]")
        o += reclen
        seen += 1
    print(f"  decoded {seen} records;  types={dict(types)};  max vert-index={maxidx}")
    print(f"  end offset = 0x{o:x}")


def dump_header(entries):
    """Decode entry-0 master header + entry-1 placement directory.

    See docs/PA_HEADER.md. Entry 0: used/sig/4-ptr table + a count-led typed
    object-index list of (blockID<<8)|subtype entries (byte-identical across PA
    files = the fixed slot roster). Entry 1: 57-record placement directory with a
    uint16 pointer table at +0x08.
    """
    e0, e1 = entries[0], entries[1]
    used0, sig = struct.unpack_from("<II", e0, 0)
    print(f"--- entry 0 (master header) len={len(e0)} ---")
    print(f"  used=0x{used0:x}  sig=0x{sig:08x}"
          f"{'  (OK)' if sig == 0x03072d39 else '  (UNEXPECTED)'}")
    ptrs = struct.unpack_from("<4I", e0, 8)
    names = ["A->flags", "B->list", "C->offdir", "D->vectors"]
    print("  ptr table @+0x08:", "  ".join(f"{n}=0x{p:x}" for n, p in zip(names, ptrs)))
    lst_off = ptrs[1] if ptrs[1] else 0x1c
    cnt = struct.unpack_from("<H", e0, lst_off)[0]
    print(f"  typed object-index list @0x{lst_off:x}: count={cnt}")
    items = [struct.unpack_from("<H", e0, lst_off + 2 + 2 * i)[0] for i in range(cnt)]
    # phase A = leading run of 0xNN08 block-roster entries; phase B = the rest
    na = 0
    while na < len(items) and (items[na] & 0xff) == 0x08:
        na += 1
    phaseA = items[:na]
    print(f"    phase A (subtype 0x08, block roster): {len(phaseA)} entries: "
          + " ".join(f"{v >> 8:02x}" for v in phaseA))
    rest = items[na:]
    print(f"    phase B (subtype 0x03/0x02 pairs): {len(rest)} u16: "
          + " ".join(f"{v >> 8:02x}.{v & 0xff:02x}" for v in rest[:24])
          + (" ..." if len(rest) > 24 else ""))

    used1, rcount = struct.unpack_from("<II", e1, 0)
    print(f"--- entry 1 (placement directory) len={len(e1)} ---")
    print(f"  used=0x{used1:x}  record count={rcount}")
    tab = [struct.unpack_from("<H", e1, 8 + 2 * i)[0] for i in range(rcount)]
    print("  uint16 ptr table @+0x08:", " ".join(f"{p:04x}" for p in tab))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--entry", type=int)
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("--header", action="store_true",
                    help="decode entry-0 master header + entry-1 directory (PA_HEADER.md)")
    ap.add_argument("--verts", nargs=2, metavar=("OFF", "CNT"),
                    help="decode int16 vertex array at OFF (hex ok) for CNT verts")
    ap.add_argument("--prims", nargs=2, metavar=("OFF", "CNT"),
                    help="decode primitive record stream at OFF (hex ok) for CNT records")
    args = ap.parse_args()

    entries = load_container(args.path)
    if args.header:
        print(f"=== {args.path} ===")
        dump_header(entries)
        return
    if args.entry is None:
        summary(entries)
        return
    e = entries[args.entry]
    print(f"=== {args.path} entry {args.entry} (len {len(e)}) ===")
    if args.verts:
        decode_verts(e, int(args.verts[0], 0), int(args.verts[1], 0))
    elif args.prims:
        decode_prims(e, int(args.prims[0], 0), int(args.prims[1], 0))
    else:
        dump_block_header(e, 24 if args.verbose else 12)


if __name__ == "__main__":
    main()
