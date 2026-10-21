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
- [LIVE_CHECKLIST.md](LIVE_CHECKLIST.md) — **every open item that needs the running game**,
  as concrete traps (address, what to look for, which doc claim it settles). Start here
  before a DuckStation session; append new items as RE raises them.

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
- [AUDIO_PIPELINE.md](AUDIO_PIPELINE.md) — the spoken voice lines: where the XA speech
  lives (TALK00–05 channels, STR audio tracks), the extract → transcribe → speaker-cluster
  → text-match pipeline (`tools/audio/`), the `voice_manifest.tsv` columns and how a dub
  would be re-injected.

### Missions / levels
- [MISSION_STATE_MACHINE.md](MISSION_STATE_MACHINE.md) — **start here for the mission
  runtime**: the lifecycle as one state machine (load → running → objectives complete →
  ending → exit), every state variable and its writers, the per-frame order, and the
  open points. Executable model: `re/ac1_mission`.
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
- [PERCEPTION_MODEL.md](PERCEPTION_MODEL.md) — stealth-mod keystone: what acquisition
  gates on (range only), the stock LOS helper `0x800781E8`, and the LOS-gate insertion point.
- [MISSION_DAMAGE_EVENTS.md](MISSION_DAMAGE_EVENTS.md) — scripted area damage (airstrike /
  breach): `op_f` is FX-only, segment hitscan `0x8007A334`, the 4-byte damage descriptor.
- [LINK_CABLE.md](LINK_CABLE.md) — two-console link-cable versus (ovl203): libcomb `sio:`
  setup, M/S handshake, 8-byte `'R'…'C'` packets, one-frame input lockstep, desync banners.
- [PLAYER_CONTROL.md](PLAYER_CONTROL.md) — what makes slot 0 "the player", the global
  pad, the player vtable installers, and the in-place dismount plan.

### UI / save data
- [AC1_TEXT_SYSTEM.md](AC1_TEXT_SYSTEM.md) — the stock text & menu rendering system
  (font texture, draw_char/draw_string, menu draw queue, name entry).
- [AC1_EMBLEM.md](AC1_EMBLEM.md) — player emblem memory-card save format.

### Project / build
- [PRISTINE_PROJECT.md](PRISTINE_PROJECT.md) — the clean, unmodified Ghidra project
  for the pristine USA v1.1 build.
- [CODE_ARCHAEOLOGY.md](CODE_ARCHAEOLOGY.md) — inferred TU map, style clusters A/B/C, mission-unit lineage, 202/203/204 divergences, matching priors (`tools/archaeology/`).

### Downstream consumers (not RE)
- [AC1MOD_VISION.md](AC1MOD_VISION.md) — pointer to the AC1mod companion
  viewer/toolkit project and the RE foundation it builds on.
- [AC1MOD_INTEGRATION.md](AC1MOD_INTEGRATION.md) — integration point between this
  repo's PA geometry decode and the AC1mod companion viewer.
- [`../mods/farsi/`](../mods/farsi/README.md) — the Persian (Farsi) localization mod,
  incubated in this repo until it has its own release cycle. Its docs
  (`LOCALIZATION.md`, name-entry RTL + shaper, title word-art, DATA screen) are in
  `mods/farsi/docs/`.

## Deep-dives & tooling

- [`disc_map/trace/`](../disc_map/trace/README.md) — investigation logs and the level
  tooling, with a status index (current / log / retracted / superseded).
- [`tools/`](../tools/) — extraction, PA decoders, mission/objective parsing, the
  Farsi toolchain, and the Ghidra/RE pipeline.
