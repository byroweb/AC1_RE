#!/usr/bin/env python3
"""Sub-trace E: re-examine the universal 'sliver/effect' slots (e125-e165) with the
FIXED gouraud decoder. For each entry: sub-objects, vertex total, bbox, coord
extremes, and primitive-type histogram."""
import sys, os, struct, collections
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools"))
import pa_obj


def prim_hist(block, s):
    o, n, h = s["prim_off"], 0, collections.Counter()
    while n < s["prim_cnt"] and o + 4 <= len(block):
        b1 = block[o + 1]; typ = block[o + 3] & 0xbc; reclen = 4 + b1 * 4
        if reclen < 4 or o + reclen > len(block):
            break
        h[typ] += 1
        o += reclen; n += 1
    return h


def scan(path, lo, hi):
    ents = pa_obj.load_container(path)
    print(f"=== {path} entries {lo}..{hi} ===")
    for ei in range(lo, min(hi + 1, len(ents))):
        block = ents[ei]
        reloc, subs = pa_obj.parse_subobjects(block)
        if not subs:
            print(f"  e{ei}: (no sub-objects, len {len(block)})")
            continue
        vtot = 0; bb = [None]*6; hist = collections.Counter()
        for s in subs:
            verts = pa_obj.read_verts(block, s["vtx_off"], s["vtx_cnt"])
            vtot += len(verts)
            for (x, y, z, w) in verts:
                for k, v in ((0, x), (1, y), (2, z)):
                    bb[k] = v if bb[k] is None else min(bb[k], v)
                    bb[k+3] = v if bb[k+3] is None else max(bb[k+3], v)
            hist += prim_hist(block, s)
        ext = max(abs(v) for v in bb) if bb[0] is not None else 0
        flat = ""
        if bb[0] is not None:
            dims = [bb[3]-bb[0], bb[4]-bb[1], bb[5]-bb[2]]
            if min(dims) == 0 or (max(dims) and min(d for d in dims if d) / max(dims) < 0.02):
                flat = "  FLAT/PLANAR"
        print(f"  e{ei}: {len(subs)} subs, {vtot:4d} verts, ext={ext:6d}, "
              f"prims={dict(hist)}{flat}")


if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else "GG/P0/PA00.T"
    scan(path, 125, 165)
