# LEVELS FOUND — walkable environment geometry is embedded in FDAT entry 2N+1 (2026-06-11)

After disproving "PA files = stages," offline extraction located the real level geometry.

## Where the levels live
The walkable environment for a mission is **embedded in `GG/COM/FDAT.T` entry `2N+1`**
(the per-mission *chunk stream*; same entry the scene loader `FUN_8004F508` reads at
runtime). Structure of that entry:
```
u32[0] = end offset of the geometry region (e.g. 0x209a8)
u32[1] = 0x80 (flags/count)
0x08.. = a run of consecutive PA-FORMAT geometry blocks (size-prefixed: block u32[0]=size)
         walk by block size: 0x08 -> 0x08+size -> ... until u32[0] (geom end)
<geom_end>.. = the other mission chunks (spawn table chunk 12, script, etc.)
```
These blocks use the **exact PA geometry format** already decoded by `tools/pa_obj.py`
(sub-object table + int16 vertex pool + variable primitive records). They decode with
**0% out-of-range faces** (clean) using the gouraud-fixed decoder.

## Confirmed by extraction (mission = Rescue Transport Truck, FDAT e67)
- 38 geometry blocks, ~112 verts each, **level-scale**: merged bbox X[-5760..5559]
  Y[-2500..0] Z[-5760..5760] (≈11000 wide, 2500 tall, floor at Y=0). Not part-scale
  (±150), not the sentinel effect slots (±32640).
- Each block centers near (0,-1250,0) and **overlaps** the others; their X/Z spans
  **rotate** block-to-block → each block is a level SECTION that needs a **per-block
  placement transform (rotation+translation)** to assemble. Naive raw-coord merge stacks
  them at the origin (renders as a circular blob — `fdat_e67_level_*.png`).

## Why earlier hunts failed (all now explained)
- PA files (all 72) are object/AC/MT/effect packs (part-scale + sentinel slots), 0
  level-scale blocks — confirmed by `classify_pa.py`. PA00 is the common bootstrap bundle.
- MIS.T = briefing TEXT/TIM (0 geometry). FDAT *entries themselves* aren't size-prefixed
  geometry — the geometry is **inside** entry 2N+1, after a small header.
- mxtid map (by file size in MXT registry `0x8004A2A4` word2): 0=PA00(880KB),
  1=RTIM(17MB,textures), 2=FDAT(27MB). So the runtime "container 1" reads were textures.

## Per-block placement transforms — THERE ARE NONE (resolved 2026-06-11)
Investigated per the assembly question: the blocks are **already in world coordinates**,
so no transforms are needed. Evidence:
- Single-block decode is clean (block 2 = a flat wall/floor panel, all type 0x2c quads,
  no spikes — `blk2_topdown.png`). Blocks are NOT spiky/garbage.
- Vertices are **absolute world coords** (±5000, e.g. wall edges `(-1496,-2300,2423)`/
  `(-1496,-300,2423)` = a vertical wall at a fixed X/Z), not local ±small.
- A floor-plan render (near-horizontal faces only, top-down) of the merged blocks shows a
  **real designed space**: rectangular floor sections + a central feature + a circular
  boundary (skydome) — `e67_floorplan.png`. The 3D "blob" was just a closed environment +
  skydome viewed from outside in a flat painter renderer.

**Assembly = plain concatenation of the blocks** (what `extract_level.py` already does).
Blocks all center near the origin because the space is roughly centered there, not because
they need positioning.

## Answer: can we extract all levels offline?
**YES — and it's simpler than feared (no transform recovery).** Every mission's
environment is in `FDAT.T` (one 27 MB file, entries 2N+1), in the already-decoded PA
geometry format, world-positioned. Batch-walk each FDAT odd entry's geometry region
(offset 0x08 .. u32[0]) and decode. **No per-level breakpoints.** Polish remaining:
- A proper renderer (backface cull / view-from-inside / textures) for clean visuals.
- Separate the skydome/boundary blocks from walkable geometry if desired.
- ~112 verts per panel is coarse PSX-era geometry; check whether a finer LOD set exists.

Tools: `extract_level.py` (walk+decode+OBJ), `classify_pa.py`, `scan_geometry.py`,
`render_obj.py`.
