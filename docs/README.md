# AC1_RE — Documentation Index

Reverse-engineering reference for Armored Core (SLUS-01323 v1.1). Start with
[REFERENCE.md](REFERENCE.md).

This page is the hub for the per-subsystem write-ups in `docs/`. Deeper hunt logs
and breadcrumbs live in [`disc_map/trace/`](../disc_map/trace/) (linked from the
relevant docs below), and the tooling that produced these findings lives in
[`tools/`](../tools/) (grouped by subsystem; see [`tools/README.md`](../tools/README.md)).

## Conventions

The docs use a small, consistent label vocabulary so readers can tell verified
facts from open questions:

- **CONFIRMED** — verified by disassembly, byte comparison, or live observation.
- **HYPOTHESIS** — inferred, not yet verified.
- **PARTIAL** — partially verified.
- **RESOLVED** / **SOLVED** — an open question that has been answered (a status, not
  a confidence level).
- **SUPERSEDED** — kept for history, replaced by a newer finding.
- **TODO** / **TBD** — open work.

## Contents

### Reference hub
- [REFERENCE.md](REFERENCE.md) — consolidated RE reference: functions, RAM
  addresses, the `.T` container format, FDAT entry-201, checksum, disc sectors.

### Disc / containers
- [MXT_LOADER.md](MXT_LOADER.md) — the MXT container & overlay loader behind every
  `.T` file load, the FDAT overlay system, and the trailing-checksum gotcha.
- [T_REPACK.md](T_REPACK.md) — the `.T` container **writer/repacker** (`tools/extract/t_repack.py`):
  inverse of `extract_t.py`, with TOC rebuild + checksum recompute (round-trips 90/90).
- [PA_FORMAT.md](PA_FORMAT.md) — `PA00–PA71.T` geometry asset packs: container +
  primitive format (identity under revision).
- [PA_HEADER.md](PA_HEADER.md) — PA##.T entry-0 (master header) + entry-1
  (object/placement directory).
- [PA_SLOTS.md](PA_SLOTS.md) — PA##.T fixed slot structure (cross-file empirical
  analysis of all 72 files).
- [MIS_FORMAT.md](MIS_FORMAT.md) — `GG/MS/MIS.T` mission data container (the entire
  mission text corpus + thumbnail TIMs).
- [DISC_TEXT_INVENTORY.md](DISC_TEXT_INVENTORY.md) — disc-wide inventory of every
  player-facing translatable string.

### Missions / levels
- [MISSION_SYSTEM.md](MISSION_SYSTEM.md) — mission runtime: descriptor, MT spawn,
  timer, objectives.
- [MISSION_STAGES.md](MISSION_STAGES.md) — mission → PA stage file + spawn
  block-index → geometry resolution.
- [LEVEL_LOADING.md](LEVEL_LOADING.md) — level loading & placement, from mission
  select to on-screen geometry.
- [OBJECT_STATS.md](OBJECT_STATS.md) — where the per-type and per-instance object/MT
  stats live.
- [INTERACTIONS.md](INTERACTIONS.md) — player interactions: Circle-context door/item
  use, COM dialogs, and live mission-completion ground-truth.
- [`../overlays/OVERLAY_MAP.md`](../overlays/OVERLAY_MAP.md) — FDAT overlay-swap map
  (UI / mission / link / arena) + the isolated Mission202 Ghidra import.

### Gameplay
- [COMBAT_PHYSICS.md](COMBAT_PHYSICS.md) — combat / physics / missiles RE notes.
- [RUNTIME_RAM_MAP.md](RUNTIME_RAM_MAP.md) — in-mission AC entity array, camera, and
  frame counters (with independent Zinfidel BizHawk cross-check).
- [ENTITY_AI_FRAMEWORK.md](ENTITY_AI_FRAMEWORK.md) — the shared entity/NPC array
  (player/AC/aircraft/train as one struct): +0x58 think-handler clean-freeze tool,
  faction/target fields, nearest-enemy target selection, and the entity damage route.

### Localization / UI
- [AC1_TEXT_SYSTEM.md](AC1_TEXT_SYSTEM.md) — the text & menu rendering system, plus
  the Farsi-localization changes layered on top.
- [AC1_NAME_RTL.md](AC1_NAME_RTL.md) — pilot-name entry screen right-to-left (Farsi)
  layout.
- [AC1_NAME_SHAPER.md](AC1_NAME_SHAPER.md) — runtime contextual shaping for Farsi
  pilot-name entry.
- [AC1_TITLE_TEXTURE.md](AC1_TITLE_TEXTURE.md) — Ravens' Nest category-title word-art
  → Farsi handoff.
- [AC1_DATA_SCREEN.md](AC1_DATA_SCREEN.md) — Ravens' Nest DATA screen round-trip
  proof + Farsi-localization handoff.
- [AC1_EMBLEM.md](AC1_EMBLEM.md) — player emblem memory-card save format.

### Project / build
- [PRISTINE_PROJECT.md](PRISTINE_PROJECT.md) — the clean, unmodified Ghidra project
  for the pristine USA v1.1 build.

### Companion project
- [AC1MOD_VISION.md](AC1MOD_VISION.md) — pointer to the AC1mod companion
  viewer/toolkit project and the RE foundation it builds on.
- [AC1MOD_INTEGRATION.md](AC1MOD_INTEGRATION.md) — integration point between this
  repo's PA geometry decode and the AC1mod companion viewer.

## Deep-dives & tooling

- [`disc_map/trace/`](../disc_map/trace/) — investigation logs and breadcrumbs
  (e.g. `PLACEMENT_SOLVED.md`, `mission_load_sequence.md`, `mission_stage_map.md`,
  `gouraud_slot_resolved.md`).
- [`tools/`](../tools/) — extraction, PA decoders, mission/objective parsing, the
  Farsi toolchain, and the Ghidra/RE pipeline.
