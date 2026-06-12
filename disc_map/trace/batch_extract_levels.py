#!/usr/bin/env python3
"""Batch-extract every walkable level from FDAT.T (entries 2N+1) to OBJ, classifying
faces into floor / ceiling / wall groups so ceilings can be toggled.

Ceiling heuristic (per the 'textures on the underside' hint): a near-horizontal face
whose winding normal points DOWN (front/textured side faces down -> you see it from
below) = ceiling; up-facing horizontal = floor; the rest = wall. (Convention may need
floor<->ceiling swap depending on winding; documented, easy to flip.)
"""
import sys, os, struct, math, json
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tools", "pa"))
import pa_obj

OUTDIR = os.path.join(pa_obj.HERE, "disc_map", "levels")
os.makedirs(OUTDIR, exist_ok=True)
HORIZ = 0.7   # |normal.y| above this = horizontal (floor/ceiling)


def fnormal(V, f):
    a, b, c = V[f[0]], V[f[1]], V[f[2]]
    u = (b[0]-a[0], b[1]-a[1], b[2]-a[2]); v = (c[0]-a[0], c[1]-a[1], c[2]-a[2])
    n = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
    m = math.sqrt(sum(x*x for x in n)) or 1.0
    return (n[0]/m, n[1]/m, n[2]/m)


def extract_entry(e):
    if len(e) < 64:
        return None
    geom_end = struct.unpack_from("<I", e, 0)[0]
    if not (16 < geom_end <= len(e)):
        return None
    V, faces = [], []
    off, nblk = 8, 0
    while off < geom_end and off + 12 <= len(e):
        sz = struct.unpack_from("<I", e, off)[0]
        if sz < 12 or off + sz > len(e):
            break
        b = e[off:off+sz]
        try:
            reloc, subs = pa_obj.parse_subobjects(b)
        except Exception:
            subs = []
        for s in subs:
            vs = pa_obj.read_verts(b, s["vtx_off"], s["vtx_cnt"]); base = len(V)
            V.extend((x, y, z) for (x, y, z, w) in vs)
            prims, tc, end = pa_obj.read_prims(b, s["prim_off"], s["prim_cnt"])
            nv = len(vs)
            for (typ, idx) in prims:
                idx = [i for i in idx if i < nv]
                if len(set(idx)) < 3:
                    continue
                g = [base+i for i in idx]
                if len(g) == 3:
                    faces.append(g)
                elif len(g) == 4:
                    faces.append([g[0], g[1], g[2]]); faces.append([g[1], g[3], g[2]])
        nblk += 1; off += sz
    if nblk < 3 or len(V) < 50:
        return None
    grp = {"floor": [], "ceiling": [], "wall": []}
    for f in faces:
        ny = fnormal(V, f)[1]
        if abs(ny) > HORIZ:
            # PSX Y is DOWN: floor's front face points -Y (fixed 2026-06-12,
            # verified visually in the AC1mod viewer; was flipped before)
            grp["floor" if ny < 0 else "ceiling"].append(f)
        else:
            grp["wall"].append(f)
    bb = [min(c[i] for c in V) for i in range(3)] + [max(c[i] for c in V) for i in range(3)]
    return V, grp, nblk, bb


def write_obj(path, V, grp):
    L = ["# AC1 level extract (FDAT 2N+1). groups: floor/ceiling/wall"]
    for v in V:
        L.append(f"v {v[0]} {v[1]} {v[2]}")
    for name in ("floor", "ceiling", "wall"):
        L.append(f"o {name}")
        for f in grp[name]:
            L.append("f " + " ".join(str(i+1) for i in f))
    open(path, "w").write("\n".join(L) + "\n")


def main():
    ents = pa_obj.load_container("GG/COM/FDAT.T")
    summary = []
    for ei, e in enumerate(ents):
        r = extract_entry(e)
        if not r:
            continue
        V, grp, nblk, bb = r
        mission = (ei - 1) // 2 if ei % 2 == 1 else None
        name = f"level_fdat_e{ei:03d}" + (f"_m{mission:02d}" if mission is not None else "")
        write_obj(os.path.join(OUTDIR, name + ".obj"), V, grp)
        span = (bb[3]-bb[0], bb[4]-bb[1], bb[5]-bb[2])
        summary.append(dict(entry=ei, mission=mission, blocks=nblk, verts=len(V),
                            floor=len(grp["floor"]), ceiling=len(grp["ceiling"]),
                            wall=len(grp["wall"]), span=span))
    summary.sort(key=lambda r: r["entry"])
    print(f"extracted {len(summary)} levels -> {OUTDIR}")
    print(f"{'entry':>5} {'miss':>4} {'blk':>4} {'verts':>6} {'floor':>5} {'ceil':>5} {'wall':>5}  span")
    for r in summary:
        print(f"{r['entry']:5} {str(r['mission']):>4} {r['blocks']:4} {r['verts']:6} "
              f"{r['floor']:5} {r['ceiling']:5} {r['wall']:5}  {r['span']}")
    json.dump(summary, open(os.path.join(OUTDIR, "_index.json"), "w"), indent=1)


if __name__ == "__main__":
    main()
