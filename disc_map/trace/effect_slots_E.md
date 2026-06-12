# Sub-trace E — the e125-e165 "sliver" slots, declassified (2026-06-11)

Re-examined with the FIXED gouraud decoder (item A). **The "near-degenerate sliver"
appearance was an artifact of the gouraud decode bug, not the data.** With stride-4
gouraud decoding these slots are clean geometry.

## What they are
A fixed slot family present in every PA file: **e125-e165, always 6 sub-objects each**,
holding each stage's **effects / props / sprites bank**. Content is **stage-specific**
(byte-DIFFERENT across PA00/PA20/PA40/PA60), though the slot role/shape is fixed.

Two subtypes (by coordinate extent + primitive types):

1. **Part-scale prop/effect meshes** (coord extent ~1245; most of e129-158, e163-165):
   ordinary small textured/gouraud/flat meshes, local origin, 0% extreme verts. They
   decode to recognizable objects — e137 renders as a clean ~214-face prop
   (`pa00_e137_effect.png`). Placed per-instance by mission data (like MT models), not
   merged at raw coords.

2. **Sprite / billboard objects** (coord extent 32640; e125-128, e141, e159-162):
   the ONLY slots that use the high-bit textured prim types **0xa4 (tri) / 0xac (quad)**.
   ~4-6% of their vertices sit at the sentinel **±0x7F80 (32640)**, mixed into otherwise
   part-scale geometry — i.e. billboard/sprite anchor vertices (likely muzzle flashes,
   explosions, lock-on/HUD-in-world sprites). The sentinel 0x7F80 marks the billboard
   vertices; the rest is normal geometry.

Note e159-165 duplicate e125-131 (same vert/prim counts) — a second bank (LOD or
texture variant).

## Primitive-type legend (byte3 & 0xBC) seen in these slots
32=0x20 flat tri · 36=0x24 tex tri · 40=0x28 flat quad · 44=0x2c tex quad ·
52=0x34 gouraud tri · 60=0x3c gouraud quad · 164=0xa4 / 172=0xac = high-bit textured
(sprite/billboard).

## Implication for AC1mod (Phase 3)
- The part-scale effect meshes are real geometry — the viewer can render them as objects
  (placed per-instance), not skip them.
- The sprite/billboard slots (ext=32640) should still be excluded from raw-coord scene
  merge (scene_mesh's coord_limit already drops them); render them as billboards/sprites
  if/when texture decode lands, or skip.
- OPEN: decode the 0xa4/0xac high-bit textured prim layout (stride currently unverified
  in PRIM_VERTS) and the 0x7F80 sentinel's exact billboard semantics — a smaller follow-up.

Tool: `disc_map/trace/sliver_scan.py`.
