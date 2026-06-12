#!/usr/bin/env python3
"""Extract the walkable LEVEL geometry embedded in an FDAT mission chunk stream
(entry 2N+1): a run of consecutive PA-format geometry blocks from offset 8 up to the
geometry-region end (entry u32[0]). Merge at raw (world) coords -> OBJ + stats."""
import sys, os, struct
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools"))
import pa_obj

def extract(fdat_entry_index, out_obj):
    ents = pa_obj.load_container("GG/COM/FDAT.T")
    e = ents[fdat_entry_index]
    geom_end = struct.unpack_from("<I", e, 0)[0]      # u32[0] = end of geometry region
    print(f"FDAT entry {fdat_entry_index}: len={len(e)}  geom region 0x08..0x{geom_end:x}")
    verts_all, faces_all = [], []
    off, nblk = 8, 0
    bb = [None]*6
    while off < geom_end and off + 12 <= len(e):
        size = struct.unpack_from("<I", e, off)[0]
        if size < 12 or off + size > len(e):
            break
        block = e[off:off+size]
        try:
            reloc, subs = pa_obj.parse_subobjects(block)
        except Exception:
            subs = []
        base = len(verts_all)
        for s in subs:
            vs = pa_obj.read_verts(block, s["vtx_off"], s["vtx_cnt"])
            sb = len(verts_all)
            for (x, y, z, w) in vs:
                verts_all.append((x, y, z))
                for k, v in ((0, x), (1, y), (2, z)):
                    bb[k] = v if bb[k] is None else min(bb[k], v)
                    bb[k+3] = v if bb[k+3] is None else max(bb[k+3], v)
            prims, tc, end = pa_obj.read_prims(block, s["prim_off"], s["prim_cnt"])
            nv = len(vs)
            for (typ, idx) in prims:
                idx = [i for i in idx if i < nv]
                if len(set(idx)) < 3:
                    continue
                g = [sb + i for i in idx]
                if len(g) == 3:
                    faces_all.append(g)
                elif len(g) == 4:
                    faces_all.append([g[0], g[1], g[2]]); faces_all.append([g[1], g[3], g[2]])
        nblk += 1
        off += size
    lines = ["# FDAT level extract"]
    for v in verts_all:
        lines.append(f"v {v[0]} {v[1]} {v[2]}")
    for f in faces_all:
        lines.append("f " + " ".join(str(i+1) for i in f))
    open(out_obj, "w").write("\n".join(lines)+"\n")
    print(f"  blocks={nblk}  verts={len(verts_all)}  tris={len(faces_all)}")
    if bb[0] is not None:
        print(f"  bbox X[{bb[0]}..{bb[3]}] Y[{bb[1]}..{bb[4]}] Z[{bb[2]}..{bb[5]}]"
              f"  span=({bb[3]-bb[0]},{bb[4]-bb[1]},{bb[5]-bb[2]})")
    print(f"  wrote {out_obj}")

if __name__ == "__main__":
    idx = int(sys.argv[1]) if len(sys.argv) > 1 else 67
    extract(idx, os.path.join(pa_obj.HERE, "disc_map", "trace", f"fdat_e{idx}_level.obj"))
