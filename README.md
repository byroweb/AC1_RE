# AC1_RE

Reverse-engineering notes, tooling, and a **Persian (Farsi) localization** for the
PlayStation game *Armored Core* (1997).

This repository targets the **North American release, `SLUS-01323` (v1.1)**. Other
regions/revisions are not supported and addresses will not match.

> **This project ships no game code, assets, or data files.** It is original source
> code and documentation only (documentation may include screenshots and derived
> analysis imagery for illustration — see [Legal](#legal)). To use the localization
> you must supply your own legally obtained copy of the game.

## What's here

| Path | Description |
| --- | --- |
| `re/` | **Behavioral C reference models** — portable, host-side, *mutable* re-implementations of game subsystems (verified against your extracted data). Flagship: `re/ac1_mxt` (container + overlay + checksum). See `re/README.md` |
| `tools/` | All tooling, grouped by subsystem (see below) |
| `tools/farsi/` | Persian localization toolchain — glyph baking, texture atlas, runtime shaping, plus the C patch sources (`font_render.c`, `string_render.c`, `farsi_name_*.c`, `farsi_table.h`, `psx_shim.h`) |
| `tools/extract/` | Disc/container extraction + text inventory (`extract_t.py`, `extract_overlay.py`, `extract_fdat.py`, `build_filemap.py`, `scan_text.py`) |
| `tools/pa/` | PA geometry-pack decoders (`pa_obj.py`, `pa_parse.py`, `pa_slots.py`) |
| `tools/mission/` | Mission/objective + spawn-table parsing (`mission_parse.py`, `mission_stages.py`, `object_stats.py`, …) |
| `tools/ghidra/` | Decompile/RE pipeline + emulator automation (`ghidra_overlay.sh` headless decompiler, `overlay2c.sh`, `setup_re_tools.sh`, `ac1_mcp_input.py`, …) |
| `ghidra_scripts/` | Ghidra scripts (Java + Jython) — overlay import, headless decompile/xref |
| `docs/REFERENCE.md` | **RE reference** — functions, addresses, `.T` format, checksum, sectors |
| `docs/` | Subsystem write-ups (start at the [documentation index](docs/README.md)) — `LEVEL_LOADING.md` (mission→geometry pipeline, placement SOLVED), `MXT_LOADER.md`, `COMBAT_PHYSICS.md`, `MISSION_SYSTEM.md`, `AC1_TEXT_SYSTEM.md`, … |

## How it works (safe distribution model)

This project **never redistributes the game's code, assets, or data files**. Instead:

1. You provide your own legally-owned disc image of `SLUS-01323`.
2. The tooling here extracts the data it needs *from your copy, on your machine*.
3. Code changes are compiled and applied as a **patch** to your own image — the
   repository ships only the changes, never the original bytes.

A MIPS `mipsel` toolchain is required to build the C patches. Addresses, offsets,
and table indices documented here are factual interoperability information.

## Legal

This is an unofficial, non-commercial fan project for the purposes of
**localization, interoperability, and education**. It is **not affiliated with,
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
