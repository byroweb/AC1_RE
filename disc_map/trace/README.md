# `disc_map/trace/` — investigation logs and level tooling

Hunt logs from the June-2026 level/PA investigation, plus the scripts that came out of
it. The logs are kept as a record of *how* things were found (including the wrong
turns); the conclusions live in `docs/`. Read the status column before trusting a file.

| File | Status | What it is / where the conclusion lives |
| --- | --- | --- |
| `PLACEMENT_SOLVED.md` | **current** | Level-section placement is translation-only (chunk 7, 52-B records). Conclusion folded into [`docs/LEVEL_LOADING.md`](../../docs/LEVEL_LOADING.md). |
| `gouraud_slot_resolved.md` | **current** | Gouraud 0x34/0x3c vertex stride = 4, live-traced. Folded into [`docs/PA_FORMAT.md`](../../docs/PA_FORMAT.md). |
| `effect_slots_E.md` | **current** | The e125+ "sliver" slots are clean geometry (the sliver look was the gouraud bug). Open: the 0xa4/0xac prim layout, slot→object mapping ([`docs/PA_SLOTS.md`](../../docs/PA_SLOTS.md)). |
| `block_binding_trace.md` | **current** | Spawn `hw3` → block record → geometry, live (training mission). Folded into [`docs/MISSION_STAGES.md`](../../docs/MISSION_STAGES.md) Part 2. |
| `mission_load_sequence.md` | **current** | The first 10 `.T` entry reads of a mission load, live. Feeds [`docs/MISSION_SYSTEM.md`](../../docs/MISSION_SYSTEM.md) §1. |
| `stage_loader_investigation.md` | **log** | Narrowing where the walkable stage comes from, after the stage-byte theory fell. Answered by `LEVELS_FOUND.md` → `PLACEMENT_SOLVED.md`. |
| `LEVELS_FOUND.md` | **log (updated)** | Walkable geometry is in FDAT entry `2N+1`. The per-block rotation speculation in it was wrong — see its banner and `PLACEMENT_SOLVED.md`. |
| `PLACEMENT_INVESTIGATION.md` | **log (solved)** | The placement hunt; the "4-orientation LUT" turned out to be the lighting table. See its banner. |
| `subresource_loader_C.md` | **log (context-corrected)** | Code trace is accurate but it is the PA00 *bootstrap* loader, not stage assembly. Open: "mission → these 9 bytes" (= `LIVE_CHECKLIST.md` S-4 / MIS-11). |
| `mission_stage_map.md` | **RETRACTED** | "Missions reuse PA00 differing only in sub-resource bytes" — wrong; kept only for the retraction record. |

## Scripts

| Script | Status | Purpose |
| --- | --- | --- |
| `assemble_levels.py` | **current** | Build every walkable level (56) as world-coordinate OBJ + top-down PNG + index, from `fdat_extracted.T`. The level pipeline. |
| `extract_level.py` | current | Single-level extraction at raw (world) coords; used by `assemble_levels.py`. |
| `render_obj.py` | current | Quick top-down + ¾ PNG of an OBJ (PIL only). Not a viewer — the interactive viewer is AC1mod (`tools/ac1mod/`). |
| `classify_pa.py` | current | Classify every PA file's blocks: part-scale / effect / level-candidate. |
| `scan_geometry.py` | current | Find PA-format geometry blocks inside any `.T` container (how the levels were found). |
| `gouraud_validate.py` | current | Validate the stride-4 gouraud decode against all PA files (0 out-of-range). |
| `sliver_scan.py` | current | Sub-trace E: re-examine e125–e165 with the fixed decoder. |
| `parse_tables.py` | current | Parse the live block-record (`0x8019F538`) + instance (`0x8019FAB8`) tables dumped from RAM. |
| `batch_extract_levels.py` | **superseded** | Pre-placement batch extractor (floor/ceiling/wall classification). Use `assemble_levels.py`. |

All outputs are derived from game data and gitignored (`disc_map/levels*/`, `*.obj`, `*.png`).
Inputs: `fdat_extracted.T` at the repo root (`tools/extract/extract_fdat.py`) or `$AC1_BIN`.
