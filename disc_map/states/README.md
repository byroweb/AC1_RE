# Save-state library (`disc_map/states/`)

The record of every DuckStation save state the RE depends on. **Docs cite state
IDs from `STATES.tsv`, never DuckStation slot numbers.**

## Why

DuckStation (and its MCP server) only has numbered slots 1–10 per game
(`<DuckStation data>/savestates/SLUS-01323_<N>.sav`). Earlier sessions wrote "slot 1 =
level start" into the docs; later sessions saved over slot 1, and by 2026-06 no in-mission
state survived (`docs/SAVESTATE_FORMAT.md` "Findings", `docs/LIVE_CHECKLIST.md` S-1).

So there are two layers:

| layer | what | where | committed? |
| --- | --- | --- | --- |
| **slots** | scratch. DuckStation/MCP writes them; anything may overwrite them. | `…/DuckStation/savestates/SLUS-01323_N.sav` | no |
| **library** | the archived `.sav` files + JSON sidecar + thumbnail, one stable ID each | `~/dev/ac1_states` (env `AC1_STATES_DIR`; **must be outside the repo**) | no — `.sav` is game RAM |
| **manifest** | one TSV row per archived state: ID, sha256, what it is, what it serves | `disc_map/states/STATES.tsv` | **yes** — contains no game data |

The tool is [`tools/duckstation/states.py`](../../tools/duckstation/states.py)
(format reader: `tools/duckstation/savestate.py`, see `docs/SAVESTATE_FORMAT.md`).

## ID convention

`{context}-{phase}-{seq}`

| part | values |
| --- | --- |
| `context` | `M00`…`M58` (mission number as the game counts it) · `GAR` garage/menus · `ARN` arena · `TTL` title/boot · `DBF` debrief |
| `phase` | `load` (NOW LOADING / before player control) · `ctrl` (first frames of player control, PA loader done) · `mid` (mid-mission, say what in *purpose*) · `obj` (objective just triggered) · `exit` (exit border / fade-out) · `dead` (player destroyed) · `main` (the context's main screen, for non-mission contexts) |
| `seq` | two digits, `01` upward; a *new* state at the same point gets the next seq, it never reuses one |

Examples: `M01-ctrl-01`, `M00-exit-02`, `GAR-main-01`, `ARN-load-01`.

Rules:
- An ID is immutable once in `STATES.tsv`. A re-capture gets a new seq (or `--replace`,
  which marks the old row `superseded` and keeps it).
- `parent` records which state a new one was played forward from (so a chain of
  `M01-ctrl-01 → M01-mid-01 → M01-exit-01` is reproducible).
- `status`: `good` (use it) · `stale` (still loads, but something it was meant to show
  no longer holds — say what in *notes*) · `superseded` (keep for history, use the newer one).
- `serves`: the `LIVE_CHECKLIST.md` IDs the state is a setup for (`S-1,LVL-1`).

## Workflow

```bash
# 1. In DuckStation: play to the moment, save to any slot (say 3).
# 2. Archive it under an ID (reads mission/stage/flags from RAM, writes sidecar + png):
python tools/duckstation/states.py archive 3 M01-ctrl-01 --purpose "first frame of control, M01" --serves S-1,LVL-1

# 3. Commit disc_map/states/STATES.tsv. Cite M01-ctrl-01 in docs / the Results log.

# Later sessions:
python tools/duckstation/states.py list --mission 1            # what exists
python tools/duckstation/states.py stage M01-ctrl-01 5         # put it in slot 5, then load slot 5 in DuckStation
python tools/duckstation/states.py check                       # sha256 every row, flag missing/orphans
```

`stage` refuses to overwrite a slot whose current content is not itself archived
(`--force` to discard it) — so a slot you forgot to archive is never silently lost.

A `.sav` that was copied by hand (into the library or anywhere else) is registered with
`adopt <path> <id> [--slot N]`: it is renamed/moved to the canonical name, dated by its
mtime, and gets the same automatic fields.

## `STATES.tsv` columns

Automatic (filled by `archive`; `?` = could not be read, see caveats below):

| column | source |
| --- | --- |
| `id` `status` `date` `file` `sha256` | the archive operation (`file` is the name inside the library) |
| `state_version` | DUCCS header `+0x04` |
| `duckstation_build` | installed emulator exe version + `settings.ini` `LastVersion` hash |
| `frame` | emulator frame counter from the System component |
| `mission` | byte `0x800411F8` |
| `stage` | PA-pack/stage byte `0x8004121B` (0 = menu/garage) |
| `objobj` | objective-object pointer `0x8019F51C` |
| `flags` | mission flags word `0x8019F524` |
| `result` | result code `0x80048610` (read as u32) |
| `in_mission` | `objobj != 0 or stage != 0` (`SaveState.in_mission()`) |
| `disc` | first 12 hex of the sha256 of the disc image (`$AC1_BIN` / `--disc`) |

Manual (`archive` flags, or edit the TSV): `purpose`, `serves`, `parent`, `caveats`, `notes`;
`status` via `set-status`. `refresh [id]` re-reads the automatic columns from the files
(after a reader fix, or once a disc / zstd decoder is available) and leaves the manual ones.

Caveats: RAM fields need the disc image (for the EXE anchor that locates RAM in the
blob) **and** a zstd decoder (`pip install zstandard`, or the `zstd` CLI). Without
either, `archive` still copies the file and records the header, with the rest as `?`.

## What is never committed

The library directory and everything in it (`.sav`, `.json`, `.png`, `disc_cache.json`).
`.gitignore` ignores `*.sav` repo-wide as a second line of defence; keep `AC1_STATES_DIR`
outside the repo (the tool refuses a library path inside it).
