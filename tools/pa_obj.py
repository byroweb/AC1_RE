#!/usr/bin/env python3
"""
pa_obj.py — Wavefront-OBJ exporter for AC1 PA##.T stage geometry blocks.

Turns a PA geometry block into a viewable .obj mesh, validating the per-sub-object
vertex-pool layout and the custom primitive decode end-to-end.

KEY STRUCTURE (RE'd from relocation walker FUN_800574D8 @0x800574D8, overlay entry
202, base 0x8004ADA0 — see docs/PA_FORMAT.md §"Sub-object table"):

  A geometry block (entry 2..N) is loaded to a buffer; the walker is called with
      a0 = block + block[+8]                       (block[+8] is a small offset)
  i.e. the SUB-OBJECT TABLE lives at  block + block[+8] + 12,  its entry count is
      *(u32)(block + block[+8] + 8).
  Each sub-object descriptor is **28 bytes**:

      off  size  field
      +0   u32   VERTEX-POOL offset   (relocation base = block+block[+8]+12)
      +4   u32   VERTEX COUNT
      +8   u32   2nd-pool offset (colour/normal pool, ×16 stride) — relocated
      +0x0e u16  FLAGS:  0x8000 = terminate/skip this sub-object
                         0x4000 = skip (no relocation) ;  low 9 bits = count addend
      +0x10 u32  PRIMITIVE-STREAM offset (relocated) — this is the walk start `t0`
      +0x14 u16  PRIM count base
      +0x18 u32  4th pool offset (relocated)

  Number of primitive records walked = (u16@+0x14) + (FLAGS & 0x1ff) - 1.
  All in-file offsets are RELATIVE TO  block+block[+8]+12  (== block + block[+8] + 12).
  Vertex indices in each primitive record are POOL-RELATIVE to that sub-object's
  vertex pool (so each sub-object is exported as its own OBJ group `o sub<N>`).

Verified on PA00 entry 2 (3 sub-objects: 122/14/44 verts; prim walks land exactly on
section boundaries 0x1218 / 0x1a38 / 0x1f34; every index < pool size).

Usage:
  python3 tools/pa_obj.py GG/P0/PA00.T --entry 2
  python3 tools/pa_obj.py GG/P0/PA00.T --entry 2 --sub 0 -o disc_map/pa00_e2_s0.obj
  python3 tools/pa_obj.py GG/P2/PA20.T --entry 3 -o disc_map/pa20_e3.obj
"""
import json, struct, sys, argparse, collections, os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")
RAW, OFF, DATA = 2352, 24, 2048

# Per primitive type (byte[3] & 0xBC): (first vertex-index byte offset, #verts, stride)
# RE'd LIVE from the relocation-walker (FUN_800574D8) per-type handlers (jump table
# 0x8004B184), disassembled from the running game (training mission, 2026-06-11) and
# calibrated against the confirmed textured-tri 0x24. In each handler `a1 = record+4`;
# halfwords shifted `<<4` (×16) are VERTEX-position indices (transformed pool, stride
# 16); `<<3` (×8) are normal/colour-pool indices (stride 8). The `<<4` positions give
# the vertex offsets below.
#   - flat/textured types pack vertex indices CONTIGUOUSLY (stride 2).
#   - GOURAUD 0x34/0x3c interleave [normal_idx, vertex_idx] pairs → vertex stride 4.
# CONFIRMED (0 out-of-range on PA00 e2 + PA20 e3): 0x20,0x28,0x24,0x2c.
# RESOLVED 2026-06-11 (was the 5-30% OOR bug — see disc_map/trace/gouraud_slot_resolved.md):
# gouraud verts are stride-4 interleaved, not contiguous; old (0x14, n) read normal
# indices as vertices → out of range.
PRIM_VERTS = {
    0x20: (0x0a, 3, 2),   # flat tri    (skip per-poly counter hw) CONFIRMED
    0x28: (0x0a, 4, 2),   # flat quad   (skip per-poly counter hw) CONFIRMED
    0x24: (0x12, 3, 2),   # textured tri (flat normal)   len 24    CONFIRMED
    0x2c: (0x16, 4, 2),   # textured quad (flat normal)            CONFIRMED
    0x34: (0x12, 3, 4),   # gouraud tri  (per-vtx normal interleaved)  CONFIRMED live
    0x3c: (0x16, 4, 4),   # gouraud quad (per-vtx normal interleaved)  CONFIRMED live
    0xa0: (0x0a, 3, 2), 0xa8: (0x0a, 4, 2),
    0xa4: (0x12, 3, 2), 0xac: (0x16, 4, 2),
    0xb4: (0x12, 3, 2), 0xbc: (0x12, 4, 2),  # high-bit textured variants (stride unverified)
}
TEXTURED = lambda t: bool(t & 0x80) or t in (0x24, 0x2c, 0x34, 0x3c)


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
    return [blob[offs[i] * 2048: offs[i + 1] * 2048] for i in range(n)]


def parse_subobjects(block):
    """Return (reloc_base, [subobj dicts]) decoded per the relocation walker."""
    if len(block) < 12:
        return 0, []
    a0 = struct.unpack_from("<I", block, 8)[0]          # block[+8]
    reloc = a0 + 12                                       # walker relocation base
    if reloc + 8 > len(block):
        return reloc, []
    count = struct.unpack_from("<I", block, a0 + 8)[0]   # a0[+8] = sub-object count
    tbl = a0 + 12
    subs = []
    for i in range(count):
        o = tbl + i * 28
        if o + 28 > len(block):
            break
        f0, vcnt, f8 = struct.unpack_from("<3I", block, o)
        flags = struct.unpack_from("<H", block, o + 0x0e)[0]
        prim_off = struct.unpack_from("<I", block, o + 0x10)[0]
        prim_cnt_base = struct.unpack_from("<H", block, o + 0x14)[0]
        if flags & 0x8000:
            continue                                     # terminated sub-object
        subs.append(dict(
            index=i,
            vtx_off=f0 + reloc, vtx_cnt=vcnt,
            pool2_off=f8 + reloc,
            flags=flags,
            prim_off=prim_off + reloc,
            prim_cnt=prim_cnt_base + (flags & 0x1ff) - 1,
        ))
    return reloc, subs


def read_verts(block, off, cnt):
    pts = []
    for i in range(cnt):
        p = off + i * 8
        if p + 8 > len(block):
            break
        x, y, z, w = struct.unpack_from("<4h", block, p)
        pts.append((x, y, z, w))
    return pts


def read_prims(block, off, cnt):
    """Walk `cnt` variable-length records; return list of (type, [vertex indices])."""
    o = off
    out = []
    types = collections.Counter()
    n = 0
    while n < cnt and o + 4 <= len(block):
        b1 = block[o + 1]
        typ = block[o + 3] & 0xbc
        reclen = 4 + b1 * 4
        if reclen < 4 or o + reclen > len(block):
            break
        info = PRIM_VERTS.get(typ)
        if info:
            voff, nv, stride = info
            idx = [struct.unpack_from("<H", block, o + voff + stride * k)[0]
                   for k in range(nv) if o + voff + stride * k + 2 <= len(block)]
            out.append((typ, idx))
            types[typ] += 1
        o += reclen
        n += 1
    return out, types, o


def export(block, subs_filter, out_path):
    reloc, subs = parse_subobjects(block)
    if subs_filter is not None:
        subs = [s for s in subs if s["index"] == subs_filter]
    lines = []
    vbase = 0          # running OBJ vertex base (OBJ is 1-indexed, global)
    total_v = total_f = 0
    bbox = [None] * 6  # minx,miny,minz,maxx,maxy,maxz
    type_counts = collections.Counter()
    deg = oor = 0
    lines.append("# AC1 PA geometry export (tools/pa_obj.py)")
    for s in subs:
        verts = read_verts(block, s["vtx_off"], s["vtx_cnt"])
        prims, tc, end = read_prims(block, s["prim_off"], s["prim_cnt"])
        type_counts += tc
        lines.append(f"o sub{s['index']}")
        for (x, y, z, w) in verts:
            lines.append(f"v {x} {y} {z}")
            for k, val in ((0, x), (1, y), (2, z)):
                bbox[k] = val if bbox[k] is None else min(bbox[k], val)
                bbox[k + 3] = val if bbox[k + 3] is None else max(bbox[k + 3], val)
        nv = len(verts)
        for (typ, idx) in prims:
            idx = [i for i in idx if i < nv]
            if len(idx) < 3:
                oor += 1
                continue
            if len(set(idx)) < 3:
                deg += 1
                continue
            g = [vbase + i + 1 for i in idx]
            if len(g) == 3:
                lines.append(f"f {g[0]} {g[1]} {g[2]}")
                total_f += 1
            elif len(g) == 4:
                # PSX 4-pt polys use Z/N order (diagonal = v1-v2): (0,1,2)+(1,3,2)
                lines.append(f"f {g[0]} {g[1]} {g[2]}")
                lines.append(f"f {g[1]} {g[3]} {g[2]}")
                total_f += 2
        vbase += nv
        total_v += nv
    with open(out_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    return dict(subs=subs, total_v=total_v, total_f=total_f, bbox=bbox,
                types=type_counts, degenerate=deg, out_of_range=oor)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--entry", type=int, required=True)
    ap.add_argument("--sub", type=int, default=None, help="export only this sub-object")
    ap.add_argument("-o", "--out", default=None)
    args = ap.parse_args()

    ents = load_container(args.path)
    block = ents[args.entry]
    stem = os.path.splitext(os.path.basename(args.path))[0]
    out = args.out or os.path.join(HERE, "disc_map",
                                   f"{stem}_e{args.entry}"
                                   + (f"_s{args.sub}" if args.sub is not None else "")
                                   + ".obj")
    if not os.path.isabs(out):
        out = os.path.join(HERE, out)

    reloc, subs = parse_subobjects(block)
    print(f"=== {args.path} entry {args.entry} (len {len(block)}) ===")
    print(f"reloc base = block+0x{reloc:x};  {len(subs)} sub-object(s)")
    for s in subs:
        print(f"  sub{s['index']:2d}: vtx@0x{s['vtx_off']:04x} x{s['vtx_cnt']:<4d}"
              f" prim@0x{s['prim_off']:04x} x{s['prim_cnt']:<4d}"
              f" flags=0x{s['flags']:04x}")
    info = export(block, args.sub, out)
    bb = info["bbox"]
    print(f"\nwrote {out}")
    print(f"  vertices: {info['total_v']}   faces (tris): {info['total_f']}")
    print(f"  prim types: {dict(info['types'])}")
    if bb[0] is not None:
        print(f"  bbox X[{bb[0]}..{bb[3]}] Y[{bb[1]}..{bb[4]}] Z[{bb[2]}..{bb[5]}]"
              f"  size=({bb[3]-bb[0]},{bb[4]-bb[1]},{bb[5]-bb[2]})")
    if info["degenerate"] or info["out_of_range"]:
        print(f"  WARN degenerate={info['degenerate']} out-of-range={info['out_of_range']}")


if __name__ == "__main__":
    main()
