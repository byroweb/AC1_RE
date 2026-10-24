# AC1_RE

Reverse-engineering reference and tooling for the PlayStation game *Armored Core*
(1997): file formats, the mission/level pipeline, the text and entity systems, and
behavioral C models of the engine's subsystems.

This repository targets the **North American release, `SLUS-01323` (v1.1)**. The
earlier North American pressing **`SCUS-94182` (v1.0)** carries **byte-identical
code**: its executable (`SCUS_941.82`, SHA-1 `ee884a2cb3ce30bc13255142fecd5a2f02ba04ad`,
the same hash as our `SLUS_013.23`), the four `FDAT.T` program overlays (201–204) and
all 58 mission overlays match ours byte for byte (verified 2026-10-18). Only data
files or disc mastering can differ between the two pressings, so every address here
applies to both, and the addresses, symbol names and layout facts in the
[Wyrelade/Armored-Core-Decomp](https://github.com/Wyrelade/Armored-Core-Decomp)
matching decompilation of v1.0 apply here directly (we borrow its facts, not its
source). Other regions (Japanese `SLPS`, European `SCES`) are not supported and their
addresses will not match.

> **This project ships no game code, assets, or data files.** It is original source
> code and documentation only (documentation may include screenshots and derived
> analysis imagery for illustration — see [Legal](#legal)). Everything here operates
> on your own legally obtained copy of the game.

## What's here

| Path | Description |
| --- | --- |
| `docs/` | **The RE write-ups.** Start at the [documentation index](docs/README.md); the master reference is [`docs/REFERENCE.md`](docs/REFERENCE.md) (functions, RAM map, `.T` container format, checksum, disc sectors). Subsystem docs: `LEVEL_LOADING.md` (mission→geometry pipeline, placement SOLVED), `MXT_LOADER.md`, `PA_FORMAT.md`, `MISSION_SYSTEM.md`, `MISSION_SCRIPT_VM.md`, `COMBAT_PHYSICS.md`, `ENTITY_AI_FRAMEWORK.md`, `AC1_TEXT_SYSTEM.md`, … |
| `re/` | **Behavioral C reference models** — portable, host-side re-implementations of game subsystems, verified against your extracted data. Flagship: `re/ac1_mxt` (container + overlay + checksum). See [`re/README.md`](re/README.md) |
| `re/match/`, `tools/match/`, `tools/loop/` | **Matching decompilation**: C that the retail compiler turns into the game's exact bytes, plus the tools and agents that produce it. Start at [`docs/MATCHING_GUIDE.md`](docs/MATCHING_GUIDE.md) (glossary, which tool when, rules) |
| `tools/` | All tooling, grouped by subsystem — see [`tools/README.md`](tools/README.md) |
| `browser-tools/` | **Interactive explorers** you open in a browser and turn knobs on (mission-loop plant view, match viewer, PS1 scene viewer), with one launchpad: `browser-tools\launch.cmd` or `python browser-tools/launchpad/server.py --open`. See [`browser-tools/README.md`](browser-tools/README.md) |
| `tools/extract/` | Disc/container extraction + repack + text inventory (`extract_t.py`, `t_repack.py` `.T` writer/repacker, `extract_overlay.py`, `extract_fdat.py`, `build_filemap.py`, `scan_text.py`) |
| `tools/pa/` | PA geometry-pack decoders + encoder (`pa_obj.py`, `pa_parse.py`, `pa_slots.py`, `pa_encode.py`) |
| `tools/mission/` | Mission/objective, spawn-table and script-VM parsing + editing (`mission_parse.py`, `mission_stages.py`, `mission_script.py`, `spawn_edit.py`, …) |
| `tools/disc/` | Write a rebuilt `.T` back into your disc image (`reinject.py`) |
| `tools/ghidra/` | Decompile/RE pipeline + emulator automation (`ghidra_overlay.sh` headless decompiler, `overlay2c.sh`, `setup_re_tools.sh`, `ac1_mcp_input.py`, …) |
| `ghidra_scripts/` | Ghidra scripts (Java + Jython) — overlay import, headless decompile/xref |
| `disc_map/`, `overlays/` | Disc file map, investigation traces, and the FDAT overlay-swap map |

## Downstream projects

The RE here is the foundation for two mods. They are consumers of this repo, not
part of the reference itself:

| Project | Where | What |
| --- | --- | --- |
| **AC1mod** | [`tools/ac1mod/`](tools/ac1mod/README.md) | PyQt disc browser + stage/model/mission viewer and CLI, heading toward a mission/map editor. Imported from the former [AC1mod repo](https://github.com/byroweb/AC1mod); builds on the decoders in `tools/`. See [`docs/AC1MOD_VISION.md`](docs/AC1MOD_VISION.md). |
| **Farsi localization** | [`mods/farsi/`](mods/farsi/README.md) | Persian (Farsi) localization patch set — renderer patch, glyph atlas, RTL name entry + runtime shaper, hub title word-art. Incubated here for now; it will move to its own repo once the translation corpus and release pipeline exist. |

## How it works (safe distribution model)

This project **never redistributes the game's code, assets, or data files**. Instead:

1. You provide your own legally-owned disc image of `SLUS-01323`.
2. The tooling here extracts the data it needs *from your copy, on your machine*.
3. Code changes (in the mods) are compiled and applied as a **patch** to your own
   image — the repository ships only the changes, never the original bytes.

A MIPS `mipsel` toolchain is required to build the C patches. Addresses, offsets,
and table indices documented here are factual interoperability information.

## Legal

This is an unofficial, non-commercial fan project for the purposes of
**interoperability, education, and localization**. It is **not affiliated with,
endorsed by, or sponsored by** FromSoftware, Inc. or Agetec, Inc.

*Armored Core* and all related assets, names, and data are the property of their
respective copyright holders. This repository contains **only original
reverse-engineered source code and documentation** — no game executable, ROM,
disc image, textures, audio, or other copyrighted game data files are included or
distributed. You must own a legitimate copy of the game to make any use of this
project.

Documentation may include screenshots and derived analysis imagery (e.g. annotated
captures and reconstructed level renders) for illustration and education; these are
the project's own analytical output, not redistributed game asset files.

The code in this repository is released under the [MIT License](LICENSE). That
license applies to the original work in this repository only, and does not grant
any rights in the underlying game.
