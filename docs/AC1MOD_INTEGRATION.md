# AC1mod — companion project pointer

AC1mod is a **separate companion viewer project** (its own repo): a PyQt disc
browser + PS1 stage/model viewer that consumes this repository's PA##.T geometry
work. The files it references (`main.py`, `core/pa_parser.py`, `core/jpsxdec.py`,
its 3D view widget, etc.) live in that companion repo, not here.

The integration point with this repo: the companion viewer's geometry decoder is a
port of this repo's authoritative PA decode in `tools/pa/pa_obj.py` /
`tools/pa/pa_parse.py` — keep a single source of truth here and vendor/import it
there, do not fork the logic. See:

- [PA_FORMAT.md](PA_FORMAT.md) — the PA##.T container + primitive format (the decode being ported).
- [PA_HEADER.md](PA_HEADER.md) / [PA_SLOTS.md](PA_SLOTS.md) — header and slot structure.

Verification of the decode itself stays in this repo: `python3 tools/pa/pa_obj.py …`
produces a mesh whose face indices are all valid and whose shape is recognizable in
any OBJ viewer.

---

## Sync for AC1mod — changes 2026-06-15 (authoring campaign)

What AC1mod needs to pull from this repo. See [AUTHORING.md](AUTHORING.md) for the
overall pipeline and [project memory `project_ac1_authoring_pipeline`].

### 1. CRITICAL — re-vendor the PA decoder (bug fix)

The sub-object descriptor's **vertex count is `u16` at `+0x04`, NOT `u32`**; `+0x06` is
a separate `u16` param (nonzero on the "articulated" block variant, e.g. PA00 entry 60).
The old `u32` read swallowed `+0x06` and **massively over-read the vertex count**,
producing garbage meshes on every variant block (PA00 e60: 1742 bogus verts → correct
64). A `break`-on-overflow guard hid it as silent truncation, not a crash, so it passed
"0 out-of-range" checks while rendering wrong.

- Fixed here in `tools/pa/pa_obj.py` (descriptor parse) and documented in
  [PA_FORMAT.md](PA_FORMAT.md) (both descriptor tables now say u16 vtx-count + u16 param).
- **AC1mod's `core/pa_parser.py` has the same latent bug** — re-vendor the fixed read.
- Regression check after vendoring: PA00 e2 (simple) = 180 verts, PA00 e60 (variant)
  = 64 verts / 100 tris. Authoritative encoder cross-check: `tools/pa/pa_encode.py
  --selftest` = 7602/7602 byte-exact across all 72 PA files.

### 1b. NEW — textured rendering (material words decoded)

The textured-primitive material is now fully decoded ([PA_FORMAT.md](PA_FORMAT.md)
"Textured-primitive MATERIAL words"), so AC1mod's viewer can render **textured**, not
just flat/wireframe. Per textured record (`family = type & 0xFD`):
- UVs: `u_i=rec[base], v_i=rec[base+1]` for base ∈ {4,8,12,16(quad)} (0..255 texel).
- `clut=u16(rec+0x06)`, `tpage=u16(rec+0x0a)`.
- `cx=(clut&0x3f)<<4; cy=clut>>6`. `px=(tpage&0xf)*64; py=((tpage>>4)&1)*256;
  bpp=(tpage>>7)&3; abr=(tpage>>5)&3`. Texels at VRAM `(px+u, py+v)`; palette row at
  `(cx,cy)`. Vertex index offset/stride per family in the PA_FORMAT material table.
- Texture pixels: most stage pages come from a **per-stage bank** (RTIM.T covers
  UI/effect/AC pages only — confirmed); until that bank's upload is traced live, a
  viewer can read VRAM directly from an emulator savestate, or flat-shade by tpage.
- `tools/pa/pa_obj.py` does not yet emit UV/material into the OBJ — that's a small
  additive enhancement (emit `vt` + a `usemtl tpage_XX` per face) if AC1mod wants
  textured OBJ export; ask and it can be added here as the single source of truth.

### 2. New decode AC1mod can surface (read-side)

- **Mission scene = FDAT entry `2N+1` chunks**: chunk 0 = local geometry (PA-format
  blocks), chunk 7 = section placement, chunk 12 = spawns, chunk 4 = mission script.
  Tools: `tools/mission/mission_parse.py` (now prints chunk ROLE labels),
  `disc_map/trace/assemble_levels.py` (full level OBJ). Collision = the render mesh
  itself (no separate data — [COLLISION.md](COLLISION.md)), so a viewer can show
  collision = geometry directly.
- **Spawns / object types**: `tools/mission/type_catalog.py` →
  `disc_map/type_catalog{,_long}.csv`. The on-screen "type" has TWO ids: `hw3` =
  behaviour/logical type (= geom block index), `hw7` = model/resource id, `hw11` = HP
  ([ENTITY_TYPES.md](ENTITY_TYPES.md), [OBJECT_STATS.md](OBJECT_STATS.md)). A viewer
  overlay should label spawns by `hw7` (model) and show HP from `hw11`.
- **Mission scripts**: `tools/mission/mission_script.py` disassembles the chunk-4 actor
  threads (scripted spawns, lerp moves, objectives) — [MISSION_SCRIPT_VM.md](MISSION_SCRIPT_VM.md).

### 3. New write-side (if AC1mod grows an editor)

Single source of truth lives here — vendor, don't fork:
`tools/pa/pa_encode.py` (geometry edit, byte-exact), `tools/mission/spawn_edit.py`
(spawn fields), `tools/mission/mission_script.py` (`assemble()`), `tools/extract/t_repack.py`
(repack + checksum), `tools/disc/reinject.py` (write back into a `.bin`). Pipeline +
gotchas (checksum, in-place size) in [AUTHORING.md](AUTHORING.md).
