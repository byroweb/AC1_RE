# Roadmap

The one-page view of where the project is going. Each track has its own detailed
plan; this page only says what is done, what is next, and where the details are.
Update the status columns here when a phase finishes.

Status as of 2026-10-29.

## Done

- **Every game function is byte-matched in plain C.** That is 5,769 manifest rows
  over the exe, ovl201–204 and the mission overlays. None is provisional: no register
  pins or empty asm. See [MATCHING_GUIDE.md](MATCHING_GUIDE.md) and
  [MATCHING_LONG_TAIL.md](MATCHING_LONG_TAIL.md).
- The formats and subsystems in the [documentation index](README.md).

## Track A — Rebuild the game from source

Plan: [FULL_LINK_PLAN.md](FULL_LINK_PLAN.md). Goal: `make verify` rebuilds every code
unit with a matching SHA-1. A byte-provenance audit labels the origin of every
output byte, and no byte may be copied straight from the retail disc.

| # | phase | status |
|---|---|---|
| P0 | Readiness audit | ✅ [report](FULL_LINK_P0.md) |
| P1 | Exe shell: `make verify` matches the exe's SHA-1 from per-function asm | ✅ [report](FULL_LINK_P1.md) |
| P2 | Exe from source: the 131 matched game functions swapped from asm to C; matched C moves into one `src/` tree | **next** |
| P3 | Program overlays 201–204, linked in one `ld` run with the exe (the exe calls into ovl201) | |
| P4 | The 57 mission overlays (and fixes for fdat188 and the fdat074 boundary) | |
| P5 | Data and bss: retail slices → symbolised data → C, and the bss order | the long tail |
| P6 | Proof kit: CI, provenance map, no-disc build, shift build, `verify.sh <your-disc>` | |
| P7 | *(optional)* matched PsyQ library source | |

Decisions: the PsyQ library stays disassembled asm from your own disc (Q1 = B′, no
SDK needed); asset-like tables may stay symbolised data (Q2); one `src/` tree for
the link and the matcher (Q3).

## Track B — Name and comment every function

The matched C shows how each function works, but often not what it is for in the
game.

| step | what | status |
|---|---|---|
| B1 | **Play sweep:** play through the checklist in [PLAY_COVERAGE_SWEEP.md](PLAY_COVERAGE_SWEEP.md) while `coverage_tag.py` records which functions run in which situation | tooling ready; 141 functions seen so far, mostly front end and garage |
| B2 | **Naming pass** (agents), in three tiers: *confirmed* (play and code agree), *static* (clear from the code alone), *unclear*. Starts from the ~1,250 name proposals in `re/match/hints.tsv` | not started; the static tier needs no play |
| B3 | **Argument/return captures** for the unclear tier (`coverage_tag.py --capture`) | tooling ready |
| B4 | **Comments:** a short summary per function; about 711 of 2,154 C files have none | not started |

Only confirmed and static names go into `include/ac1/symbols.tsv`.

## Track C — AC1mod: mission and map editor

Plan: [AC1MOD_VISION.md](AC1MOD_VISION.md). Code: [`tools/ac1mod/`](../tools/ac1mod/README.md).

| gap | what | status |
|---|---|---|
| 1 | `.T` repacker | ✅ 90/90 containers round-trip |
| 2 | MIS.T mission editor (names, briefings, mission parameters) and full-disc reinject | **next** |
| 3 | PA map/model reader and encoder | partly done (`tools/pa/pa_encode.py`) |
| 4 | Graph linking each mission to its map, text and parameters | open |
| 5 | Packaging (toolkit and GUI) | open |

## Ongoing

- **Live checks in the emulator:** [LIVE_CHECKLIST.md](LIVE_CHECKLIST.md) confirms
  single behaviours (missions, entities, combat, levels, perception, damage,
  control) and feeds the subsystem docs.
- **Farsi localization** ([`mods/farsi/`](../mods/farsi/README.md)), a downstream
  mod incubated here.

## How the tracks connect

- B is easier after A's P2: names then land in the one `src/` tree.
- A's P5 (data as C) goes faster with B's names and the subsystem docs, because
  each table needs a type and a name.
- C uses the format decoders in `tools/` and, later, names from B.
