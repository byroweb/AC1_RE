#!/usr/bin/env python3
"""Assemble AC1 walkable levels from FDAT.T with REAL world placement (SOLVED 2026-06-12).

Where everything lives (FDAT entry 2N+1 = per-mission chunk stream [u32 len][payload]):
  chunk 0       : the level geometry BLOCKS (PA-format, size-prefixed, LOCAL coords,
                  PRE-ROTATED on disc — no per-section rotation needed)
  chunk 7 @ +0  : the SECTION PLACEMENT TABLE — 52-byte records, terminated by s16 -1 @ +6:
      +0x00 s16[3]  P0   world bbox MIN of the placed section
      +0x06 s16     index (PVS/cell); -1 terminates the table
      +0x08 s16[3]  P1   world bbox MAX
      +0x10 s16[3]  P2   PLACEMENT TRANSLATION (world = block_local + P2)
      +0x18 s16[3]  copy of P0
      +0x1e s16     GEOMETRY BLOCK INDEX into chunk 0 (blocks are REUSED -> instancing)
      +0x26 s16     per-section small index (zone/portal)
      +0x28 u8      lighting-record index (104-byte lighting table, fog+colors+
                    4 pre-rotated light matrices; built at load by 0x80056958)
      +0x2a u16     flags (runtime word +0x30 = flags<<16 | light_idx)

Runtime ground truth (Nassau Tunnel / Rescue Transport Truck, live-traced):
  table expanded to 64-byte records @ 0x801D0B68; block ptrs @ 0x801A6344 (38);
  level renderer: visibility walk (caller of 0x800639D8) -> stack list of 24-byte
  visible records -> per-section GTE setup @ 0x80063A60 (rt from 0x801AD6CC + k*32,
  4 camera-composed variants; llm from lighting record + k*20) -> vertex loop
  0x80063B9C (lwc2/rtps; composed matrix is 1/8-scaled to fit big level coords).

Output: disc_map/levels_assembled/level_eNNN_mMM.obj (floor/ceiling/wall groups,
WORLD coordinates) + _topdown.png + _index.json. All outputs are game-derived ->
gitignored, never distribute.
"""
import sys, os, struct, math, json, colorsys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools"))
import pa_obj

OUTDIR = os.path.join(pa_obj.HERE, "disc_map", "levels_assembled")
os.makedirs(OUTDIR, exist_ok=True)
HORIZ = 0.7


def chunks_of(e):
    out, off = [], 0
    while off + 4 <= len(e):
        ln = struct.unpack_from("<I", e, off)[0]
        if ln == 0 or off + 4 + ln > len(e):
            break
        out.append((off + 4, ln))
        off += 4 + ln
    return out


def blocks_of(e):
    geom_end = struct.unpack_from("<I", e, 0)[0]
    blocks, off = [], 8
    while off < geom_end:
        sz = struct.unpack_from("<I", e, off)[0]
        if sz < 12 or off + sz > len(e):
            break
        blocks.append(e[off:off + sz])
        off += sz
    return blocks


def placements_of(e, nblocks):
    ch = chunks_of(e)
    if len(ch) < 8:
        return []
    toff, tlen = ch[7]
    out = []
    for i in range(tlen // 52):
        r = e[toff + i * 52: toff + i * 52 + 52]
        if len(r) < 52:
            break
        f6, = struct.unpack_from("<h", r, 6)
        if f6 == -1:
            break
        p2 = struct.unpack_from("<3h", r, 0x10)
        blk, = struct.unpack_from("<h", r, 0x1e)
        light = r[0x28]
        if not (0 <= blk < nblocks):
            continue
        out.append((blk, p2, light))
    return out


def fnormal(V, f):
    a, b, c = V[f[0]], V[f[1]], V[f[2]]
    u = (b[0]-a[0], b[1]-a[1], b[2]-a[2]); v = (c[0]-a[0], c[1]-a[1], c[2]-a[2])
    n = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
    m = math.sqrt(sum(x*x for x in n)) or 1.0
    return (n[0]/m, n[1]/m, n[2]/m)


def block_mesh(b):
    """Decode one PA block -> (verts, faces) in LOCAL coords."""
    try:
        reloc, subs = pa_obj.parse_subobjects(b)
    except Exception:
        return [], []
    V, F = [], []
    for s in subs:
        vs = pa_obj.read_verts(b, s["vtx_off"], s["vtx_cnt"])
        base = len(V)
        V.extend((x, y, z) for (x, y, z, w) in vs)
        prims, tc, end = pa_obj.read_prims(b, s["prim_off"], s["prim_cnt"])
        nv = len(vs)
        for (typ, idx) in prims:
            idx = [i for i in idx if i < nv]
            if len(set(idx)) < 3:
                continue
            g = [base + i for i in idx]
            if len(g) == 3:
                F.append(g)
            elif len(g) == 4:
                F.append([g[0], g[1], g[2]]); F.append([g[1], g[3], g[2]])
    return V, F


def assemble(ei, e):
    blocks = blocks_of(e)
    if len(blocks) < 2:
        return None
    plc = placements_of(e, len(blocks))
    if not plc:
        return None
    meshes = {}
    V, faces = [], []
    for (blk, p2, light) in plc:
        if blk not in meshes:
            meshes[blk] = block_mesh(blocks[blk])
        bv, bf = meshes[blk]
        base = len(V)
        V.extend((x + p2[0], y + p2[1], z + p2[2]) for (x, y, z) in bv)
        faces.extend([base + i for i in f] for f in bf)
    if len(V) < 50:
        return None
    grp = {"floor": [], "ceiling": [], "wall": []}
    for f in faces:
        ny = fnormal(V, f)[1]
        if abs(ny) > HORIZ:
            # PSX Y is DOWN: a walkable floor's front face points -Y (world up).
            # (Verified in the AC1mod viewer 2026-06-12 — earlier convention was flipped.)
            grp["floor" if ny < 0 else "ceiling"].append(f)
        else:
            grp["wall"].append(f)
    return V, grp, len(blocks), len(plc)


def write_obj(path, V, grp):
    L = ["# AC1 level, ASSEMBLED with world placement (chunk-7 table). groups: floor/ceiling/wall"]
    for v in V:
        L.append(f"v {v[0]} {v[1]} {v[2]}")
    for name in ("floor", "ceiling", "wall"):
        L.append(f"o {name}")
        for f in grp[name]:
            L.append("f " + " ".join(str(i + 1) for i in f))
    open(path, "w").write("\n".join(L) + "\n")


def topdown_png(path, V, grp):
    try:
        from PIL import Image, ImageDraw
    except ImportError:
        return
    xs = [v[0] for v in V]; zs = [v[2] for v in V]
    minx, maxx, minz, maxz = min(xs), max(xs), min(zs), max(zs)
    W = 1000
    S = (W - 40) / max(maxx - minx + 1, maxz - minz + 1)
    img = Image.new("RGB", (W, W), (8, 8, 14)); dr = ImageDraw.Draw(img)
    P = lambda x, z: (20 + int((x - minx) * S), W - 21 - int((z - minz) * S))
    for name, col in (("floor", (70, 170, 90)), ("wall", (150, 150, 200)), ("ceiling", (60, 60, 80))):
        for f in grp[name]:
            pts = [P(V[i][0], V[i][2]) for i in f]
            for a in range(len(pts)):
                dr.line([pts[a], pts[(a + 1) % len(pts)]], fill=col, width=1)
    img.save(path)


def main():
    ents = pa_obj.load_container("GG/COM/FDAT.T")
    summary = []
    for ei in range(1, len(ents), 2):
        e = ents[ei]
        if len(e) < 64:
            continue
        r = assemble(ei, e)
        if not r:
            continue
        V, grp, nblk, nplc = r
        mission = (ei - 1) // 2
        name = f"level_e{ei:03d}_m{mission:02d}"
        write_obj(os.path.join(OUTDIR, name + ".obj"), V, grp)
        topdown_png(os.path.join(OUTDIR, name + "_topdown.png"), V, grp)
        xs = [v[0] for v in V]; zs = [v[2] for v in V]
        summary.append(dict(entry=ei, mission=mission, blocks=nblk, sections=nplc,
                            verts=len(V), span=(max(xs)-min(xs), max(zs)-min(zs))))
        print(f"e{ei:3} m{mission:2}: {nblk:3} blocks, {nplc:3} sections, "
              f"{len(V):6} verts, span {max(xs)-min(xs)}x{max(zs)-min(zs)}")
    json.dump(summary, open(os.path.join(OUTDIR, "_index.json"), "w"), indent=1)
    print(f"\nassembled {len(summary)} levels -> {OUTDIR}")


if __name__ == "__main__":
    main()
