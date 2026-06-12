# AC1_RE

Reverse-engineering notes, tooling, and a **Persian (Farsi) localization** for the
PlayStation game *Armored Core* (1997).

This repository targets the **North American release, `SLUS-01323` (v1.1)**. Other
regions/revisions are not supported and addresses will not match.

> **This project contains no game code, assets, or data.** It is source code and
> documentation only. To use the localization you must supply your own legally
> obtained copy of the game. See [Legal](#legal) below.

## What's here

| Path | Description |
| --- | --- |
| `re/` | **Behavioral C reference models** — portable, host-side, *mutable* re-implementations of game subsystems (verified against your extracted data). Flagship: `re/ac1_mxt` (container + overlay + checksum). See `re/README.md` |
| `font_render.c` | Reconstruction of the glyph renderer (`draw_char`) |
| `string_render.c` | Reconstruction of the string renderer (`draw_string`) |
| `farsi_name_shape.c` / `farsi_name_input.c` | Persian shaping + name-entry handling |
| `farsi_table.h` | Persian glyph mapping table |
| `psx_shim.h` | Minimal PSY-Q/libps type & macro shims for building patches |
| `farsi_*.py` | Glyph baking, texture atlas, and runtime-shaping tooling |
| `tools/` | Extraction/repack + RE tooling (`extract_overlay.py`, `extract_t.py`, `ghidra_overlay.sh` headless decompiler, …) |
| `extract_fdat.py`, `patch_draw_string.py` | Container extraction & patch build helpers |
| `ghidra_scripts/` | Ghidra scripts (Java + Jython) — overlay import, headless decompile/xref |
| `REFERENCE.md` | **RE reference** — functions, addresses, `.T` format, checksum, sectors |
| `docs/` | Subsystem write-ups — `LEVEL_LOADING.md` (mission→geometry pipeline, placement SOLVED), `MXT_LOADER.md`, `COMBAT_PHYSICS.md`, `MISSION_SYSTEM.md`, … |
| `AC1_TEXT_SYSTEM.md` | Documentation of the game's text/menu system |

## How it works (safe distribution model)

This project **never distributes any part of the game**. Instead:

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
disc image, textures, audio, or other copyrighted game data is included or
distributed. You must own a legitimate copy of the game to make any use of this
project.

The code in this repository is released under the [MIT License](LICENSE). That
license applies to the original work in this repository only, and does not grant
any rights in the underlying game.
