# AC1mod — companion project pointer

AC1mod is a **separate companion viewer/toolkit project** (its own repo) that
consumes this repository's reverse-engineering and tooling. The goal of that
project is to read/edit AC1 missions and maps, view models, and re-pack the disc.

This repo provides the format-RE foundation it builds on:

- `docs/PA_FORMAT.md` / `docs/PA_HEADER.md` / `docs/PA_SLOTS.md` — PA##.T geometry
  containers, headers, and the cross-file slot skeleton.
- `docs/MIS_FORMAT.md` / `docs/MISSION_SYSTEM.md` / `docs/MISSION_STAGES.md` —
  mission text, names, and stage/PA binding.
- `docs/LEVEL_LOADING.md` — level section placement and assembly.
- `tools/extract/` — disc file map (`build_filemap.py` → `disc_map/disc_files.json`),
  the `.T` container reader (`extract_t.py`), and text tooling.
- `tools/pa/` — the authoritative PA geometry decoders (`pa_obj.py`, `pa_parse.py`,
  `pa_slots.py`); the companion viewer vendors/imports this decode rather than
  forking it.

Keep the authoritative format decoders here in `tools/`; the companion AC1mod
project reuses them.
