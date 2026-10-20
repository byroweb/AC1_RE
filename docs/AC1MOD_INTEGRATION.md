# AC1mod — integration with `tools/`

AC1mod, the PyQt disc browser and PS1 stage/model/mission viewer, now lives in this
repo at [`tools/ac1mod/`](../tools/ac1mod/README.md). It was imported 2026-10-19
from the former separate repo ([byroweb/AC1mod](https://github.com/byroweb/AC1mod)
at `616bcd2`), and that repo keeps its history. Paths below such as `main.py` and
`core/pa_parser.py` are relative to `tools/ac1mod/`.

AC1mod has no format decoders of its own. It imports the authoritative ones from
`tools/` (`core/_tools.py` puts their directories on `sys.path`) and keeps only
adapters that reshape their output for the viewer:

| AC1mod module | decodes with | what AC1mod adds |
|---|---|---|
| `core/pa_parser.py` | `tools/pa/pa_obj.py` (`parse_subobjects`, `subobject_tris`: vertex pools, primitive records, flat RGB / textured `(tpage, clut)`, Z/N quad split) | `Mesh`/`Face` objects, `material_color`, contact sheet, world-space stage |
| `core/pa_parser.read_container` | `tools/extract/extract_t.read_sectors` + `tools/extract/t_repack.unpack` | the sector range comes from the jPSXdec index |
| `core/fdat.py` | `read_container` above | FDAT.T sector lookup, mission N -> entries 2N / 2N+1, tree labels |
| `core/mis.py` | `tools/extract/extract_t.read_sectors` | the MIS.T preview-TIM scan |
| `core/xa.py` | `tools/audio/xa_adpcm.py` (`stream_sound_areas`, `decode_stream_float`, mono + stereo) | `core/xa.c`, a bit-identical C port of `decode_stream_float` for fast preview (checked on all 43.8M samples of the disc's TALK and ACED streams) |

A format fix goes into `tools/` only; AC1mod picks it up on the next run. See:

- [PA_FORMAT.md](PA_FORMAT.md) — the PA##.T container + primitive format (the decode being ported).
- [PA_HEADER.md](PA_HEADER.md) / [PA_SLOTS.md](PA_SLOTS.md) — header and slot structure.

Verification of the decode itself stays in this repo: `python3 tools/pa/pa_obj.py …`
produces a mesh whose face indices are all valid and whose shape is recognizable in
any OBJ viewer.

---

## Sync for AC1mod — changes 2026-06-15 (authoring campaign)

What AC1mod needs to pull from this repo. See [AUTHORING.md](AUTHORING.md) for the
overall pipeline and [project memory `project_ac1_authoring_pipeline`].

### 1. CRITICAL — PA decoder vertex-count fix

The sub-object descriptor's **vertex count is `u16` at `+0x04`, NOT `u32`**; `+0x06` is
a separate `u16` param (nonzero on the "articulated" block variant, e.g. PA00 entry 60).
The old `u32` read swallowed `+0x06` and **massively over-read the vertex count**,
producing garbage meshes on every variant block (PA00 e60: 1742 bogus verts → correct
64). A `break`-on-overflow guard hid it as silent truncation, not a crash, so it passed
"0 out-of-range" checks while rendering wrong.

- Fixed here in `tools/pa/pa_obj.py` (descriptor parse) and documented in
  [PA_FORMAT.md](PA_FORMAT.md) (both descriptor tables now say u16 vtx-count + u16 param).
- AC1mod decodes through `tools/pa/pa_obj.py`, so it has the fix:
  `ac1mod_cli.py info GG/P0/PA00.T` reports e2 = 180 verts / 323 faces.
- Regression check: PA00 e2 (simple) = 180 verts, PA00 e60 (variant)
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
- `tools/pa/pa_obj.py` decodes each face's `(tpage, clut)` (`prim_shading`,
  `read_prims_shaded`, `subobject_tris`) but its OBJ export does not emit UVs or
  materials yet. Textured OBJ export (`vt` + a `usemtl tpage_XX` per face) would be
  a small addition there.

**Material-map render (done in AC1mod 2026-06-15):** until real texels are available,
AC1mod now colours each textured face by a stable hue derived from its decoded
`(tpage, clut)` (`core/pa_parser.material_color`, over `pa_obj`'s decoded material), exposed as `ac1mod_cli.py level N
--materials`. Distinct textures read as distinct flat colours — a faithful *texture-layout*
view, not real texels. **CORRECTION (2026-10-12): the texels have NOT been located.** The
"entry 0 = encoded texture bank, entry 1 = CLUT directory" claim was retracted — those
entries are the PA master header and the placement directory ([PA_HEADER.md](PA_HEADER.md);
reconciliation in [PA_FORMAT.md](PA_FORMAT.md)). Do not build an entry-0 texture decoder.
Real texturing waits on a live `LoadImage` trace / an in-mission VRAM dump
([LIVE_CHECKLIST.md](LIVE_CHECKLIST.md) LVL-2/LVL-3; [SAVESTATE_FORMAT.md](SAVESTATE_FORMAT.md) —
the current backups are all garage).

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

Single source of truth lives here — import, don't copy:
`tools/pa/pa_encode.py` (geometry edit, byte-exact), `tools/mission/spawn_edit.py`
(spawn fields), `tools/mission/mission_script.py` (`assemble()`), `tools/extract/t_repack.py`
(repack + checksum), `tools/disc/reinject.py` (write back into a `.bin`). Pipeline +
gotchas (checksum, in-place size) in [AUTHORING.md](AUTHORING.md).
