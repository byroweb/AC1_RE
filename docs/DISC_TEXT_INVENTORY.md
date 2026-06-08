# AC1 Disc-Wide Translatable-Text Inventory

Target: **SLUS-01323 (v1.1)**, pristine image. Produced by the pure-RE session of
2026-06-08. Goal: locate **every** player-facing string on the disc so the Farsi
translation can proceed systematically instead of one screen at a time.

## TL;DR — where the text is

| Container | Disc sectors | Translatable text | Renderable by `draw_string`? |
| --- | --- | --- | --- |
| **`GG/MS/MIS.T`** | 101920–103449 | **Mission names + briefings + objectives** (~65k chars, 50 missions) | ✅ yes (100% ASCII) |
| **`GG/COM/FDAT.T`** entry 201 | 72189–85330 | **UI/menus/part-shop/memory-card prompts** (~16k chars) | ✅ yes |
| `GG/P0–P3/PA*.T` (72 files) | 103637–126704 | **none** (map geometry/textures only) | n/a |
| `GG/MS/MENU_TIM.T` | 100295–101035 | word-art **textures** (GARAGE/MISSION/MAIL/SHOP/SYSTEM/RANKING) | image, not text |
| STR/XA/BGM media | 124+ | none (FMV/audio) | n/a |

**Totals:** `disc_map/text_inventory.csv` — **2,658 strings, ~81k chars, 100%
flagged renderable** by the existing renderer. That means the Farsi
`draw_string` path (`string_render.c`) can display translated MIS/FDAT text
**with no new renderer** — only the bytes change (+ checksum recompute, §6).

## Deliverables (this session)
- `disc_map/disc_files.json` — exact file map (133 files) from the jPSXdec `.idx`,
  re-pointed at the **pristine** backup image.
- `tools/build_filemap.py` — `.idx` → file map (validated vs `REFERENCE.md` §9).
- `tools/extract_t.py` — generalized `.T` extractor (auto-detects count-first /
  offset-first TOC, tolerates zero-length entries, dumps per-entry payloads).
- `tools/scan_text.py` — `draw_string`-aware text scanner; **passes the FDAT-201
  ground-truth gate** (re-finds `NEW GAME`, `MISSION`, `RANKING`, `SYSTEM`;
  correctly *omits* GARAGE/MAIL/SHOP which are textures).
- `disc_map/text_inventory.csv` — the inventory.
- `disc_map/MIS_T_text.txt`, `disc_map/FDAT_T_text.txt` — human-readable dumps for
  translators.
- `docs/MIS_FORMAT.md`, `docs/PA_FORMAT.md` — cracked container layouts.

## Mission text ↔ mission number (linkage)
No code table needed — the mapping is structural (confirmed in `MIS.T`):
- `MIS.T` **entry 0** = mission-name array, **0x20-byte stride**, first at +0x100;
  **array index = mission number** (`name[m]` at `0x100 + m*0x20`).
- Each mission's briefing/objective text is a contiguous block of `MIS.T` entries
  beginning with a `Requester:` entry (50 blocks, 16/32-entry stride).
- *(Open / Phase F):* the live `read_T_entry(MIS, …)` caller and which on-disk
  mission index a given sortie loads — confirm via Ghidra/DuckStation when wiring
  the actual MIS translation patch. Not required to *translate*, only to verify.

## Encoding cheatsheet (for translators / shapers)
- Page terminator `>` (0x3E); final-screen terminator `/` (0x2F).
- Line breaks: literal CR `0x0D` / LF `0x0A`.
- Colour/format markup: literal `@d` (highlight, `draw_string` palette escape) and
  `%d` (MIS column/format). Preserve these verbatim through shaping.
- All text is ASCII; for Farsi, shape with `farsi_runtime_shape.py` and store the
  draw-order glyph bytes in-place per entry (same model as the name buffer).

## Reproduce
```sh
python3 tools/build_filemap.py
python3 tools/extract_t.py --file GG/MS/MIS.T --toc-only
python3 tools/scan_text.py  --toc disc_map/extracted/MIS_T/toc.json  --csv disc_map/text_inventory.csv
python3 tools/extract_t.py --file GG/COM/FDAT.T --toc-only
python3 tools/scan_text.py  --toc disc_map/extracted/FDAT_T/toc.json --terminated-only --append --csv disc_map/text_inventory.csv
```
