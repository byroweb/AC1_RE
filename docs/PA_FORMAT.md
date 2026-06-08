# `GG/P0–P3/PA00–PA71.T` — Stage/map packs (format + text finding)

Target: **SLUS-01323 (v1.1)**. **72 files** (`PA00.T`..`PA71.T`) across dirs
`GG/P0`, `GG/P1`, `GG/P2`, `GG/P3`, disc sectors **103637–126704**, ~700–900 KB
each. Loaded via `load_T_file(3, …)` (file id 3, `REFERENCE.md` §3).

## Container format (CONFIRMED)
Standard `.T` container, **count-first** TOC:
- `PA00.T`: `uint16[0]` = **176** (entry count), offsets at `uint16[1..]`,
  many zero-length entries (sparse object slots).
- Entry *i* spans sectors `TOC[i]..TOC[i+1]-1`; 114/176 non-empty in PA00.
- Extract: `python3 tools/extract_t.py --file GG/P0/PA00.T --dump-entries`

## Contents — geometry/textures only, **NO translatable text**
Probed PA00 / PA20 / PA40 / PA60: **0 candidate English words** (regex
`[A-Za-z]{5,}` with a vowel) across the entire files. The `>`/`/`-terminated
"strings" the scanner finds are false positives from binary model/texture data.

Entry payloads are PlayStation map assets — TMD model packs, TIM textures,
collision/placement data. (Detailed sub-typing of geometry vs. collision vs.
script is **out of scope** for the translation effort and left for the future
"AC1mod" map-tooling work, see `docs/AC1MOD_VISION.md`.)

## Conclusion for translation
**Skip all 72 PA##.T files.** They hold no player-facing text. The translatable
mission prose lives in `MIS.T` (briefings/names/objectives) and `FDAT.T` entry
201 (UI/menus); see `docs/DISC_TEXT_INVENTORY.md`.
