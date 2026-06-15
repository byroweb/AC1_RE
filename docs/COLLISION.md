# AC1 collision — world & object (derived from render geometry)

Target **SLUS-01323 (v1.1)**. Static RE (offline disasm). Source notes:
`scratch/re/collision.md`. Companions: `PA_FORMAT.md` (geometry), `LEVEL_LOADING.md`
(chunk 0/7 assembly), `COMBAT_PHYSICS.md` (damage path).

## Headline (CONFIRMED)

**There is NO separate collision dataset** — no BSP, heightfield, or collision-only
chunk. Collision is **derived from the render geometry**. For authoring this is a
simplification: editing a section's render mesh edits its collision too; there is no
parallel collision mesh to keep in sync.

Two distinct systems, both reusing render data:

### 1. Object ↔ object (projectiles vs ACs/props) = swept-AABB + sphere
- Raycast entry `FUN_8006D534` → AC loop `0x80074728` (entity array `0x801A26B8`,
  stride `0x170`, 16 slots; AABB centre @`+0x08`, half-extents @`+0x6A`/`+0x6C`) +
  prop loop `0x80073598` (pointer array `0x8019F308`, count `0x8019F508`).
- Primitives (base exe): swept segment-vs-AABB `0x80012E5C`, sphere/box proximity
  `0x800126D8`. Per-frame broadphase `0x8006D628`/`0x8006D980` tests a query point vs
  ACs + props + section origins. **No faces** in this path.

### 2. World / terrain (AC-vs-wall/floor, projectile-vs-level) = per-face polygon
- Public swept API `0x80070F30` (`a0`=startPos, `a2`=resultBuf, `sp+0x30`=flag-group
  mask) — called by AC movement (`0x800710B8`/`0x80071164`/`0x800748C4`) and projectiles
  (`0x800832C8`/`0x800834B0`/`0x80084CCC`).
- Narrowphase `0x8006F4D8`: bit-grid broadphase `0x8006FB6C` (per-cell u32 bitmask of
  section indices; set/clear `0x8006DB9C`/`0x8006DBE4`) → per-section bbox cull
  `0x8006EFFC` → orientation transform (`k = sec & 3`, the 4 render variants) → per-face
  loop `0x8006F08C` → polygon test `0x8006EE8C`.
- The face test resolves vertices via `index & 0x7FF8` (= idx×8) into the block's int16
  `(x,y,z,flag)` **stride-8 vertex pool** — the exact PA render-geometry format
  (`PA_FORMAT.md`). **The render mesh IS the collision mesh.**

## Data source / record format (CONFIRMED)
- **Section placement table `0x801D0B68`** (64-B runtime records, expanded from chunk-7
  52-B records): `+0x06` terminator(-1), `+0x0E` `flags&7` = collision group, `+0x10`
  translation/origin, `+0x18`/`+0x20` bbox P0/P1, **`+0x1E` = geometry block index**
  (same field the renderer uses), `+0x30` bit `0x20000` = enable.
- **Block descriptor table `0x80091228`** (8-B; `+0x04` = geom ptr). Geometry block:
  `lhu@+0` = face count, **face stride = 20 bytes** (`faceEnd = start + count*5*4`),
  vertex stride 8.
- Chunk 0 (geometry) relocated by `0x80053848` → `0x801A5F34`; the same blocks feed
  render and collision. Chunk 7 supplies placement.

## Disputed (2026-06-15 — confirm live)
This static recon reads `0x8004F024` / `0x8004EF74` as a timed value-easing routine
(no world reads, no GTE), i.e. NOT the movement integrator. **This conflicts with the
live finding in `COMBAT_PHYSICS.md`** (which calls `0x8004F024` the AC movement
integrator, invoked from the enemy update wrapper `0x800760DC`). One reading is
mis-scoped — resolve in DuckStation (breakpoint `0x8004F024`; does it write `ac+0x08`
from `ac+0x64`?). Either way it is separate from world collision, which is
`0x80070F30` → `0x8006F4D8` (above).

## Authoring implication
A new/edited stage needs only its **render geometry + placement** correct; collision
follows automatically. The flag-group mask (`section +0x0E flags&7`, queried via the
`0x80070F30` `sp+0x30` arg) selects which sections collide for a given query — the lever
for non-colliding decoration vs solid walls.

## Open / confirm-live
- The exact site that builds the occupancy bit-grid at scene load (set helpers known;
  the filling caller not pinned).
- Whether the 20-byte level face stores a precomputed plane (vs the 8-byte object prim) —
  byte cross-check against an assembled chunk-0 block.
- Best next: breakpoint `0x8006EE8C` while walking into a wall; confirm the resolved
  verts are the chunk-0 PA verts of the section's `+0x1E` block, validating the 20-B face.
