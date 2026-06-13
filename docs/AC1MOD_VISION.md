# AC1mod — companion project pointer

AC1mod is a **separate companion viewer/toolkit project** (its own repo) that
consumes this repository's reverse-engineering and tooling. The goal of that
project is to read/edit AC1 missions and maps, view models, and re-pack the disc.

This repo provides the format-RE foundation it builds on:

- [PA_FORMAT.md](PA_FORMAT.md) / [PA_HEADER.md](PA_HEADER.md) / [PA_SLOTS.md](PA_SLOTS.md) — PA##.T geometry
  containers, headers, and the cross-file slot skeleton.
- [MIS_FORMAT.md](MIS_FORMAT.md) / [MISSION_SYSTEM.md](MISSION_SYSTEM.md) / [MISSION_STAGES.md](MISSION_STAGES.md) —
  mission text, names, and stage/PA binding.
- [LEVEL_LOADING.md](LEVEL_LOADING.md) — level section placement and assembly.
- `tools/extract/` — disc file map (`build_filemap.py` → `disc_map/disc_files.json`),
  the `.T` container reader (`extract_t.py`), and text tooling.
- `tools/pa/` — the authoritative PA geometry decoders (`pa_obj.py`, `pa_parse.py`,
  `pa_slots.py`); the companion viewer vendors/imports this decode rather than
  forking it.

Keep the authoritative format decoders here in `tools/`; the companion AC1mod
project reuses them.

## Gaps to close for "rip & author missions"
1. **`.T` writer / repacker** — ✅ **DONE: `tools/extract/t_repack.py`** (see
   [T_REPACK.md](T_REPACK.md)). Inverse of `extract_t.py`: rebuilds a `.T` from
   edited entries, recomputes per-entry checksums, preserves the TOC convention +
   terminator quirk. Round-trips **90/90** containers byte-identical; a worked
   `MIS.T` text edit proves the path. Remaining for full disc edits: raw 2352-B
   sector reframing + `psxinject` reinject (the grow-past-span case needs an
   image rebuild).
2. **MIS.T mission editor** — names (entry-0 array) + briefing/objective text
   (per-mission blocks) + thumbnail TIMs. Text path is solved; needs an
   edit/repack UI and the mission-load linkage (`read_T_entry(MIS,…)` caller →
   on-disk mission index, and the mission *parameter* tables in lower FDAT
   entries: enemy spawns, map id, rewards, time limit).
3. **PA##.T map/model reader** — sub-type the geometry entries (TMD model packs,
   TIM textures, collision, object placement). TMD is a documented PSX format;
   KFModTool already does AC model/texture — borrow its TMD/TIM parsing rather
   than redo it.
4. **Mission ↔ map ↔ asset graph** — which `PA##.T` + `MIS.T` entry + FDAT
   parameter row a given mission uses (so a "new mission" knows what to reference).
5. **Packaging** — extend an existing PSXmod-style project with AC1-aware plugins,
   ship a standalone Python toolkit wrapping these `tools/` scripts + a small
   GUI/CLI, or drive the live-verify loop through the Ghidra/DuckStation MCP. The
   format-RE layer is provider-agnostic.

Build order: the `.T` repacker (gap 1) is done; next is the MIS.T mission editor
(gap 2) plus full-disc reinject, then the PA block encoder (gap 3) for authoring
geometry.
