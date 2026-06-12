#!/usr/bin/env python3
"""Classify every PA file's geometry blocks to find which (if any) hold level-scale
environment geometry vs part-scale common assets. Per block: part (span<4000),
sentinel/effect (ext>30000, the 0x7F80 billboard slots), or LEVEL-candidate
(span>=4000 and ext<=30000 = large coherent world geometry)."""
import sys, os, struct, json
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools"))
import pa_obj

fm = json.load(open(os.path.join(pa_obj.HERE, "disc_map", "disc_files.json")))
pa_paths = sorted(r["path"] for r in fm["files"]
                  if (r.get("path") or "").upper().endswith(".T") and "/PA" in (r.get("path") or "").upper())

print(f"{'file':16} {'geomBlk':>7} {'part':>5} {'effect':>6} {'LEVEL':>5}  level-candidate blocks (entry:verts,span)")
for p in pa_paths:
    ents = pa_obj.load_container(p)
    part = effect = level = 0
    cands = []
    for ei, block in enumerate(ents):
        if len(block) < 16 or struct.unpack_from("<I", block, 0)[0] != len(block):
            continue
        reloc, subs = pa_obj.parse_subobjects(block)
        if not subs:
            continue
        vtot = 0; bb = [None]*6
        for s in subs:
            for (x, y, z, w) in pa_obj.read_verts(block, s["vtx_off"], s["vtx_cnt"]):
                vtot += 1
                for k, v in ((0, x), (1, y), (2, z)):
                    bb[k] = v if bb[k] is None else min(bb[k], v)
                    bb[k+3] = v if bb[k+3] is None else max(bb[k+3], v)
        if vtot < 8 or bb[0] is None:
            continue
        ext = max(abs(v) for v in bb)
        span = max(bb[3]-bb[0], bb[4]-bb[1], bb[5]-bb[2])
        if ext > 30000:
            effect += 1
        elif span >= 4000:
            level += 1
            cands.append(f"e{ei}:{vtot}v,{span}")
        else:
            part += 1
    blk = part + effect + level
    ctxt = "  ".join(cands[:6]) if cands else ""
    print(f"{p.split('/')[-1]:16} {blk:7} {part:5} {effect:6} {level:5}  {ctxt}")
