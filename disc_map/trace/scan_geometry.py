#!/usr/bin/env python3
"""Scan a .T container for PA-geometry-format blocks (sub-object tables / vertex pools).
Reports entries that decode as geometry, with vertex totals and bbox extent — to find
where the level/environment geometry lives. World-scale bbox (big extent) = candidate
level; part-scale (~hundreds) = AC/MT/object.
"""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools"))
import pa_obj


def scan(path, limit_entries=None):
    try:
        ents = pa_obj.load_container(path)
    except SystemExit as e:
        print(f"  !! {path}: {e}")
        return
    print(f"=== {path}: {len(ents)} entries ===")
    hits = 0
    for ei, block in enumerate(ents):
        if limit_entries and ei >= limit_entries:
            break
        if len(block) < 16:
            continue
        # PA geometry block heuristic: u32[0] == len(block) (size-prefixed)
        import struct
        sz = struct.unpack_from("<I", block, 0)[0]
        size_match = (sz == len(block))
        if not size_match:
            continue                      # require the size-prefix signature (real PA geom)
        try:
            reloc, subs = pa_obj.parse_subobjects(block)
        except Exception:
            subs = []
        if not subs:
            continue
        vtot = 0; bb = [None]*6
        for s in subs:
            verts = pa_obj.read_verts(block, s["vtx_off"], s["vtx_cnt"])
            vtot += len(verts)
            for (x, y, z, w) in verts:
                for k, v in ((0, x), (1, y), (2, z)):
                    bb[k] = v if bb[k] is None else min(bb[k], v)
                    bb[k+3] = v if bb[k+3] is None else max(bb[k+3], v)
        if vtot < 8 or bb[0] is None:
            continue
        ext = max(abs(v) for v in bb)
        span = (bb[3]-bb[0], bb[4]-bb[1], bb[5]-bb[2])
        scale = "LEVEL?" if ext > 4000 or max(span) > 4000 else "part"
        hits += 1
        print(f"  e{ei:3d}: len={len(block):6d} size_ok={int(size_match)} "
              f"{len(subs):2d} subs {vtot:5d} verts  ext={ext:6d} span={span} [{scale}]")
    print(f"  -> {hits} geometry-like entries")


if __name__ == "__main__":
    paths = sys.argv[1:] or ["GG/COM/FDAT.T", "GG/MS/MIS.T"]
    for p in paths:
        scan(p)
        print()
