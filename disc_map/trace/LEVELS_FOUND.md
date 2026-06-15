# LEVELS FOUND — walkable environment geometry is embedded in FDAT entry 2N+1 (2026-06-11)

> **UPDATE 2026-06-12: placement SOLVED — see [PLACEMENT_SOLVED.md](PLACEMENT_SOLVED.md).** Sections are
> placed by TRANSLATION ONLY (chunk 7 of the same entry, 52-byte records); blocks are
> pre-rotated on disc and reused (instancing). `assemble_levels.py` builds all 56 levels
> with correct world layout. The "rotation+translation per block" speculation below was
> half right (translation yes, per-section rotation no).

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
These blocks use the **exact PA geometry format** already decoded by `tools/pa/pa_obj.py`
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

## Per-block placement transforms — THEY EXIST (corrected 2026-06-11)
**CORRECTION:** an earlier draft here said "no transforms / world-positioned" — WRONG
(user ground-truth: the real level is an X with curved legs, not the circle the naive
merge produces). The blocks ARE local and need a per-block transform.

Evidence:
- All 38 block **centroids sit at the origin** (e.g. blk2 (32,-896,-89), blk5 (88,-896,
  -33)) — the sections are stacked on top of each other, not tiled. A per-block colored
  render shows every block radiating from the same center (`e67_perblock.png`).
- The render path proves it: cull/transform `FUN @ 0x80057C44` loads a per-block PSX
  `MATRIX` — **rotation** (5 packed words @ `0x800b1228`) + **translation** (3 words @
  `0x800b123c`) — before drawing each block. So placement = **rotation + translation**.
- Entry 2N+1 is a proper `[u32 len][payload]` **chunk stream** (not `[geom_end][blocks]`):
  chunk 0 = geometry (the 38 blocks), chunk 1 (107 KB) + chunk 2 = geometry-handler
  siblings (`FUN_80053848`), … chunk 12 = the object/MT **spawn** table.
- The section transforms are NOT in the spawn table (chunk 12 places objects — truck/MTs —
  at ±17000 with block indices 0/1, sub-trace D style). They come from the geometry
  chunks (0–2), source TBD (candidate: chunk 1).

**OPEN / next:** capture the per-block matrices. Cleanest = runtime: while standing in a
level, write-watch `0x800b1228` (+ `0x800b123c`) and collect all ~38 (rotation,translation)
pairs in one frame, then apply them to assemble the X and verify. Then locate that data in
the chunk stream (chunk 1?) to extract all levels offline.

## Answer: can all levels be extracted offline?
**YES — and it's simpler than feared (no transform recovery).** Every mission's
environment is in `FDAT.T` (one 27 MB file, entries 2N+1), in the already-decoded PA
geometry format, world-positioned. Batch-walk each FDAT odd entry's geometry region
(offset 0x08 .. u32[0]) and decode. **No per-level breakpoints.** Polish remaining:
- A proper renderer (backface cull / view-from-inside / textures) for clean visuals.
- Separate the skydome/boundary blocks from walkable geometry if desired.
- ~112 verts per panel is coarse PSX-era geometry; check whether a finer LOD set exists.

Tools: `extract_level.py` (walk+decode+OBJ), `classify_pa.py`, `scan_geometry.py`,
`render_obj.py`.
