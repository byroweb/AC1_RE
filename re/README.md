# AC1_RE — behavioral reference models

Clean, portable **C reference implementations** of Armored Core 1 (SLUS-01323,
US v1.1) subsystems, reconstructed by reverse engineering. The goal is to
*recreate the game's behavior* in code that is **readable and mutable** — for
understanding, for tooling/mods, and as a foundation for an eventual standalone
re-implementation.

## What this is (and isn't)

- **Behavioral models, not a matching decompilation.** The code here does *not*
  compile to byte-identical MIPS. It re-implements the same *behavior* in plain
  C99, with the game's real addresses/struct offsets recorded as named constants
  so each model stays anchored to the binary (see the `docs/` write-ups).
- **Verified against the real data where possible.** Models compile and run, and
  are checked against artifacts extracted from a disc you own (e.g. the MXT model
  validates every overlay checksum in `fdat_extracted.T`).
- **Mutable for modding.** Tables and constants are exposed and editable; helpers
  like `ac1_mxt_fix_checksum()` let you change game data and repack it so the game
  still boots (the trailing-word checksum is the usual thing that bricks a naive
  edit — see `docs/MXT_LOADER.md`).

No copyrighted game code or assets are included. You supply your own disc image;
the tools/models operate on data you extract from it.

## Layout

```
re/
  include/   public headers (one per subsystem + shared ac1 types)
  src/       the reference-model implementations
  examples/  small runnable programs that exercise/validate a model
  Makefile   build + `make test` (validates against ../fdat_extracted.T if present)
```

Build: `make` (or `make test`). Requires only a C99 compiler.

## Relationship to the rest of the repo

This `re/` tree is the home for **portable, compilable, validated** reference
models. It does not replace existing work — it consolidates and extends it:

- **`../docs/REFERENCE.md`** is the master RE reference (memory map, `.T` format,
  checksum, quick address index, mission overlay). Models cite it; they don't
  duplicate its tables.
- The repo already has **patch-oriented** reconstructions in `../tools/farsi/`
  (`font_render.c`, `string_render.c`, the `farsi_name_*.c` set), built
  against `../tools/farsi/psx_shim.h` for `mipsel-gcc`. Those target the console
  (they bake into the game); the models here target the **host** (portable C99,
  run/verify on your PC). Both are "behavioral C," different deployment.
- Extraction/repack **tooling** lives in `../tools/` (`extract/extract_t.py`,
  `farsi/build_rtl_patch.py`, …) and `../tools/ghidra/ghidra_overlay.sh` (headless
  decompile of any overlay function). Models are derived from those + `docs/`.

## Models

| model        | status        | what it reproduces                                  | anchored by |
|--------------|---------------|-----------------------------------------------------|-------------|
| `ac1_mxt`    | **done, validated** | `.T`/MXT archives, FDAT overlay layout, the `0x12345678` checksum, overlay header (`entry_fn`, `"ENERGY"` magic) | `docs/MXT_LOADER.md` |
| text render  | **done** | `draw_char`/`draw_string` glyph + string rendering  | `../tools/farsi/font_render.c`, `../tools/farsi/string_render.c`, `../docs/AC1_TEXT_SYSTEM.md` |
| `ac1_mission`| planned       | mission lifecycle: driver, timer, objective vtable, success/fail → result | `docs/MISSION_SYSTEM.md` |
| `ac1_combat` | planned       | projectile pool + per-type think (ballistic/missile homing), proximity/collision, damage application | `docs/COMBAT_PHYSICS.md` |
| `ac1_physics`| planned       | AC movement integrator (boost/gravity/recoil)       | `docs/COMBAT_PHYSICS.md` |
| `ac1_parts`  | planned       | garage AC assembly: part stat tables → totals (AP/weight/EN) | (in progress) |

Each model maps 1:1 to a documented subsystem so the C and the RE notes stay in
sync.

## Validate the MXT model

```
$ make test
../fdat_extracted.T: 26914816 B, 13142 sectors, count-first, 205 entries
code overlays:
  [front_end   ] entry 201  542720 B  checksum OK
  [mission     ] entry 202  286720 B  checksum OK
  [link_vs     ] entry 203  268288 B  checksum OK
  [local_battle] entry 204  268288 B  checksum OK
overlay checksums: ALL VALID
```

## Provenance / how to extend

The models are written from the RE captured in `docs/` and the project memory.
To add one: read the matching `docs/*.md`, pull the relevant functions with
`tools/ghidra/ghidra_overlay.sh <overlay.bin> <addr>`, then translate the behavior into
a small, well-commented `.c`/`.h` pair plus an `examples/` validator.
