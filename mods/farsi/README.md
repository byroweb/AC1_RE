# Armored Core (SLUS-01323 v1.1) — Persian (Farsi) localization

A localization *mod* built on the reverse-engineering in the parent repo. It lives
under `mods/` because it is a **consumer** of the RE, not part of the reference:
the same relationship as the separate AC1mod viewer. It is incubated here until it
has a translation corpus and a release pipeline, at which point this directory
lifts out into its own repository (vendoring the few `tools/` scripts it needs).

> Bring your own disc. Nothing here contains game data; every script reads your
> legally-owned image and writes a patched copy next to it.

## Status (as of the move, 2026-10)

Engine side — **done and proven on hardware-equivalent emulation**:

- `draw_string` Farsi branch (`string_render.c`): any byte `>= 0x80` → variable-width
  Farsi glyph from the packed `fmet[]` table. Start menu renders RTL, right-aligned.
- 128-glyph atlas (`farsi_glyphs.py`, `farsi_atlas.py`, `farsi_texture.py`,
  `farsi_metrics.json` / `farsi_table.h`): 32 letters × contextual forms, lam-alef,
  Persian digits, Noto Sans Arabic Bold.
- Shaping: build-time `farsi_shape.py` ≡ reference `farsi_runtime_shape.py` ≡ R3000
  `farsi_name_shape.c` (byte-identical on the validation set).
- Pilot/AC name entry: RTL keyboard, edge-detected input, live contextual shaping,
  proportional right-justify; **survives the memory-card round-trip**
  ([docs/AC1_NAME_RTL.md](docs/AC1_NAME_RTL.md), [docs/AC1_NAME_SHAPER.md](docs/AC1_NAME_SHAPER.md)).
- Hub category titles (GARAGE/RANKING/MAIL/SYSTEM/MISSION/SHOP) as crisp Farsi
  word-art, baked and cold-boot verified ([docs/AC1_TITLE_TEXTURE.md](docs/AC1_TITLE_TEXTURE.md)).
- Persian digits in the ASCII digit cells (`build_pdigits.py`) so stock number
  rendering shows ۰–۹ unchanged.

Not started — **the translation itself**:

- No translated string corpus exists yet. The parent repo's
  [`docs/DISC_TEXT_INVENTORY.md`](../../docs/DISC_TEXT_INVENTORY.md) locates every
  player-facing string (2,658 strings / ~81k chars in `MIS.T` + `FDAT.T` entry 201),
  all renderable by the patched `draw_string` with no further engine work.
- DATA-screen labels and the rest of the menus are still English
  ([docs/AC1_DATA_SCREEN.md](docs/AC1_DATA_SCREEN.md)).
- A `strings_fa.tsv` (string id → Persian) + one `build.py` that shapes, repacks,
  reinjects and emits a patch is the deliverable that turns this into a releasable mod.

## Layout

| File | Role |
| --- | --- |
| `string_render.c`, `font_render.c`, `psx_shim.h` | draw_string / draw_char reconstructions with the Farsi branch; compile with `mipsel-gcc` |
| `farsi_name_rtl.c`, `farsi_name_input.c`, `farsi_name_shape.c`, `farsi_name_xpos.s` | name-entry keyboard, input handler, runtime shaper, right-justify |
| `farsi_glyphs.py`, `farsi_atlas.py`, `farsi_texture.py`, `farsi_metrics.json`, `farsi_table.h` | glyph inventory → atlas → metrics → packed C table |
| `farsi_shape.py`, `farsi_runtime_shape.py`, `farsi_bake.py`, `shape_data_labels.py` | shaping + baking Persian text to ROM bytes |
| `build_rtl_patch.py` | the disc bake: compile shaper, patch FDAT entry 201, fix checksum, reinsert into a copy of your image |
| `patch_draw_string.py`, `build_pdigits.py` | draw_string code injection; Persian digits into `MENU_TIM.T` |
| `title_wordart.py`, `title_build.py`, `title_wordart_rects.json` | hub title word-art render + bake |
| `docs/` | mod documentation: [`LOCALIZATION.md`](docs/LOCALIZATION.md) (engine changes), name entry, titles, DATA screen; screenshots in `docs/screens/` |

## Building (summary)

Inputs are never committed; point the scripts at your own files:

```sh
export AC1_BIN="/path/to/Armored Core (v1.1).bin"      # pristine SLUS-01323 v1.1, 2352-B sectors
python3 mods/farsi/build_rtl_patch.py                   # -> "... [RTL].bin" + .cue next to it
export AC1_RTL_BIN="/path/to/Armored Core (v1.1) [RTL].bin"
python3 mods/farsi/title_wordart.py --build && python3 mods/farsi/title_build.py
```

`build_rtl_patch.py` expects the extracted `fdat_extracted.T` at the repo root
(see `tools/extract/extract_fdat.py`) and a `mipsel` toolchain on `PATH`. Intermediate
files go to `/tmp/` (see each script's header). Full detail and the gotchas
(entry-201 trailing checksum, VRAM conflicts, where free overlay space really is)
are in [`docs/LOCALIZATION.md`](docs/LOCALIZATION.md).

## Dependencies on the parent repo

- Addresses and formats: [`docs/REFERENCE.md`](../../docs/REFERENCE.md),
  [`docs/AC1_TEXT_SYSTEM.md`](../../docs/AC1_TEXT_SYSTEM.md) (stock text system),
  [`docs/MXT_LOADER.md`](../../docs/MXT_LOADER.md) (overlay checksum).
- Tooling: `tools/extract/extract_t.py`, `tools/extract/t_repack.py`,
  `tools/disc/reinject.py`. When this directory becomes its own repo, these get
  vendored or pinned as a submodule — do not fork them.
