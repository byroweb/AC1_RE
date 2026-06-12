# AC1 Pristine RE project (`AC_1_USA_2048_RE`)

A clean, **unmodified** Ghidra project for the USA PS1 v1.1 build (SLUS-01323),
intended to be **shared** without any Farsi-translation artifacts. The Farsi
working project is now `AC_1_USA_2048_FARSI`; this one mirrors all of its
reverse-engineering knowledge against pristine bytes.

## Layout

| Block | Range | Source | Notes |
|-------|-------|--------|-------|
| `CODE` | `0x80011000–0x80039fff` | `SLUS_013_2048.23` (PS-X EXE) | Resident exe, imported via the **PSX Executables Loader** (`ghidra_psx_ldr`), language `PSX:LE:32:default`, PSYQ signatures on. |
| `overlay` | `0x80050000–0x800dffff` | `overlay_80050000.bin` | `front_end` runtime image (the menu/text/name code — `draw_string` etc.). |

The PSX loader also creates the standard PS1 hardware/RAM blocks (I/O, DMA, SPU,
CACHE, RAM). The `0x8004ada0–0x8004ffff` front_end sub-region is **not** mapped,
so a few overlay functions show `Unable to read bytes at 0x8004xxxx` warnings —
harmless.

## Overlay provenance (why it's still "pristine")

`overlay_80050000.bin` is a **DuckStation RAM dump** (2026-06-06), not an on-disc
file — the `front_end` overlay is loaded at runtime from FDAT.T entry 201. It was
**verified pristine** before inclusion:

- The Farsi patch footprint = `cmp fdat_extracted.T fdat_patched.T` → 884 bytes,
  all inside FDAT **entry 201** (`front_end`), file offsets `0x185f154–0x18e1fff`.
- Mapped to RAM (load base `0x8004ada0`), 830 of those offsets fall in the dump
  window. At **826** the dump carries the **pristine** byte, at **0** the patched
  byte; the remaining 4 are ordinary runtime data drift.
- Conclusion: the Jun-6 dump predates the patch work (Jun-7+) and contains zero
  patched bytes. See the plate comment at `0x80050000` in the project.

## Markup transfer

Knowledge is Ghidra **markup** (names, prototypes, comments) keyed by address —
not C source. Because the resident bytes are identical, it transfers 1:1:

- `ghidra_scripts/DumpMarkup.java` — exports non-default function names,
  prototypes, calling conventions, all listing comments, and user labels from
  `AC_1_USA_2048_FARSI` to a TSV (full address strings, overlay-safe).
- `ghidra_scripts/ApplyMarkup.java` — replays the TSV into this project.
- Result: **1061 functions, 1058 prototypes, 143 comments** (3 prototypes
  reference absent types — re-run after Parse C Source if you want them).
- Two Farsi-flavored names were neutralized for sharing:
  `farsi_gte_renderer_type6 → gte_renderer_type6`,
  `draw_kanji_RECLAIMED_shape_name → draw_kanji`.

## Regenerate from scratch

```bash
export XDG_CONFIG_HOME=/home/byron/snap/ghidra/current/.config   # so headless finds ghidra_psx_ldr
HL=/snap/ghidra/current/ghidra_12.0_PUBLIC/support/analyzeHeadless
cd /home/byron/Desktop/AC_1_USA_RE
rm -rf AC_1_USA_2048_RE.gpr AC_1_USA_2048_RE.rep AC_1_USA_2048_RE.lock*

# 1. dump markup from the FARSI project (read-only)
"$HL" "$PWD" AC_1_USA_2048_FARSI -process -readOnly -noanalysis \
  -scriptPath "$PWD/ghidra_scripts" -postScript DumpMarkup.java "$PWD/ac1_markup.tsv"
sed -e 's/farsi_gte_renderer_type6/gte_renderer_type6/g' \
    -e 's/draw_kanji_RECLAIMED_shape_name/draw_kanji/g' ac1_markup.tsv > ac1_markup_pristine.tsv

# 2. import pristine exe + overlay + apply markup
"$HL" "$PWD" AC_1_USA_2048_RE -import "$PWD/SLUS_013_2048.23" \
  -scriptPath "$PWD/ghidra_scripts" \
  -postScript AddRawBlockAnalyze.java "$PWD/overlay_80050000.bin" 0x80050000 overlay \
  -postScript ApplyMarkup.java "$PWD/ac1_markup_pristine.tsv"
```

> **Headless gotcha:** the PSX loader/language lives in the snap's settings dir,
> so `analyzeHeadless` (run outside the snap sandbox) only finds it when
> `XDG_CONFIG_HOME` points at `~/snap/ghidra/current/.config`. Without it you get
> `Language not found for 'PSX:LE:32:default'`. Use the **12.0** headless
> (`snap current` → 35); 12.1 lacks the extension.
