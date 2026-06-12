# Level-section PLACEMENT — investigation log (OPEN PROBLEM, 2026-06-11/12)

The walkable level geometry is found (FDAT entry 2N+1, chunk 0 = PA-format blocks, see
`LEVELS_FOUND.md`). The blocks are **local, stacked at the origin** and need a **per-section
transform (rotation + translation)** to assemble into the real layout (user ground-truth:
Rescue Transport Truck is an X with curved legs, not the circle the naive merge makes).
**The transforms have NOT been cleanly recovered.** This logs what's known + what was ruled
out so the next pass starts informed.

## CONFIRMED about placement
- It is **rotation + translation per section** (both), applied **at render time**.
- Render setup `FUN @ 0x80057EF0` (inside the level render-loop fn that ends at 0x80057F6C,
  called from 0x8005DE98): loads the GTE matrix per object —
  rotation via `ctc2 rt_0..rt_4` from `s3`; translation via `ctc2 trx/try/trz` from
  `s1+0x98` (`v0=s1+0x84; lw +0x14/+0x18/+0x1c`). Loop stride `s0 += 124`.
- The loaded GTE matrix is **camera-composed** (world×camera), recomputed every frame
  (changes as the AC moves) → camera-relative, not raw world placement.
- **Rotations resolve to a small set of 4 orientations 90° apart** (e.g.
  `[0,-4096,0; ±1960,1201,±3389; 0,∓2040,∓3551]`), all sharing up-axis `[0,-4096,0]` →
  **the 4 X arms**. Found as a LUT around `0x801ad00c` (Rescue) — GTE-packed 5-word (20B)
  rotations. So sections are rotated into 4 arm directions.

## Ruled OUT (8 approaches, all dead ends for the TRANSLATIONS)
1. **44-byte block records @ 0x8019F538** — only the 4 bootstrap/object blocks; the
   34 unexamined bytes per record are **all zeros**. Not the level sections.
2. **`FUN_80053848`** (chunk 0-2 handler) — **relocation only**: reads count, per record
   stores block ptr at `0x801a5f34`/`0x801a6020`, computes secbase, calls walker
   `0x800574d8`, advances by block size. **No transform writes anywhere in it.**
3. **Section geometry in work RAM** (ptrs at `0x801a6020` → `0x800Bxxxx`) — vertices are
   **local** (walker relocates indices, not coords). Placement not baked into geometry.
4. **Matrix table `0x801ad000` region** — has the rotation LUT (4-8 orientations, 20B
   GTE-packed) but **no clean translation field** beside them (`+20` = the matrix's
   trailing word / `0xF0000000` pad, not a translation).
5. **Display structs `0x801A2828`** (0x170×16) — the `+0x98` field read 0 / large junk;
   the cull fn's `s1` (0x801A2888) ≠ the render-loop's `s1`, so wrong field.
6. **GTE TR @ cull-fn `0x80057C44`** — gives a real composed translation (e.g.
   `(3077,-1500,13455)` for one Chrome section) but it's camera-relative AND the bp fires
   **per sub-object**, mixing AC parts / objects / sub-objects. No clean per-section run.
7. **`0x800b1228`/`0x800b123c`** (cull-fn matrix) — `+0x14` translation varies **per
   sub-object** (one section's bbox corners: X const, Y/Z drift), not section placement.
8. **Chunk 1 (107 KB) offline** — geometry-handler sibling, structure unclear; 107 KB is
   far too big for 38 transforms; likely a 2nd geometry/LOD set, not a placement table.

## Leading hypotheses for the NEXT pass
- **Per-mission objective code (FDAT entry 2N)** sets up the placement. It's relocatable
  MIPS at `0x801C4B40` (vtable); its **init method** is called after the chunk walk
  (`[*0x8019F51C + 0x14]`, per MISSION_SYSTEM). If placement is code-driven (loops applying
  the 4-orientation LUT + a translation table), the translation table is the target —
  reverse the objective object's init.
- **Find the composition** that writes the render-loop's per-object matrix each frame
  (`world×camera`). Breakpoint it, read the **static placement input** (camera-independent).
  Cleanest empirical test: read a candidate translation at two AC positions — the static
  one doesn't change; the composed one does.
- **Solve from runtime math**: capture composed matrix `M_i` per section + the camera view
  matrix `C`, then `Placement_i = C⁻¹ × M_i`. Needs the camera matrix located.

## Breadcrumbs (addresses) for the next pass
| what | addr |
|------|------|
| scene loader (mission) | `FUN_8004F508` (calls @ 0x8004F598/F5B4/F5D4/F78C) |
| `.T` entry reader | `FUN_800165E4(container,entry,dest)` |
| `.T` open-by-path | `0x80016678` (refs "\GG\", MXT reg 0x8004A2A4) |
| bootstrap PA loader (PA00) | `FUN_8004F1A8`, stage byte `0x8004121B` (NOT mission selector) |
| sub-resource loader | `~0x8004F2A0` (8 slots, entry=byte+addend) |
| geometry chunk handler | `FUN_80053848` (relocates; ptr table 0x801a5f34 / sections 0x801a6020) |
| geometry walker | `0x800574D8` (jump table 0x8004B184) |
| cull/transform (per sub-obj) | `0x80057C44` (matrix @ 0x800b1228 rot / 0x800b123c trans) |
| render-loop matrix setup | `~0x80057EF0` (ctc2 rot from s3, trans from s1+0x98) |
| primitive emitter | `0x8005A57C`; gouraud handlers 0x80057674/0x800577a4 |
| objective object (per-mission code) | FDAT 2N → `0x801C4B40` (init = `[*0x8019F51C+0x14]`) |
| mission chunk stream | FDAT 2N+1; chunks: 0=geom, 1=107KB, 2=geom, 12=spawn(256×40) |
| container mxtids | 0=PA00(880K), 1=RTIM(17M tex), 2=FDAT(27M) |

Tools: `extract_level.py`, `batch_extract_levels.py` (→ disc_map/levels/, 59 levels,
floor/ceiling/wall groups), `classify_pa.py`, `scan_geometry.py`, `render_obj.py`.
