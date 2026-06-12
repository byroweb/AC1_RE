# `GG/MS/MIS.T` — Mission data container (CRACKED)

Target: **SLUS-01323 (v1.1)**. Disc sectors **101920–103449** (1530 sectors,
3,133,440 B). Standard `.T` container ([REFERENCE.md](REFERENCE.md) §3), **count-first** TOC.

> Holds the **entire mission text corpus** — every mission name, briefing, and
> objective screen — as plain `draw_string`-renderable text. This is the single
> biggest block of player-facing prose on the disc.

## TOC
- `uint16[0]` = **884** (entry count) → **883 entries**, offsets at `uint16[1..]`.
- Entry *i* spans sectors `TOC[i]..TOC[i+1]-1` (file-relative; disc LBA = `101920 + off`).
- 547 non-empty entries: **194 TIM** images + **353 data/text** entries (+ empties).
- Extract with: `python3 tools/extract/extract_t.py --file GG/MS/MIS.T --dump-entries`

## Entry roles

| Entry | Sectors | Role |
| --- | --- | --- |
| **0** | 1–4 (4 sec) | **Mission-name table** — see below |
| 1,3,5,7,… | 1 ea | **Mission text pages** (Requester/briefing/objective), ASCII |
| (interleaved) | 6 ea | **TIM** thumbnails, 100×100 8bpp (mission map / target art) |

### Entry 0 — mission-name array
Fixed **0x20-byte stride**, first name at file offset **+0x100**:
`name[m]` at `0x100 + m*0x20`, `>`-terminated ASCII. **Index = mission number.**
51 slots (`m=0..50`); slots 16/29/39 are `"NN Omitted"` placeholders, `m=0` is
`"Dummy00"`. Full list in `disc_map/MIS_T_text.txt`.

### Per-mission text blocks
**50** mission blocks, each begins with a `"Requester:"` entry (entry indices
1, 17/33/49…, stride **16 or 32**). A block contains, in order:
1. **Reward header** — `Requester:` / `Advance:` / `Upon success:` (`>`-terminated).
2. **Briefing pages** — several 1-sector entries of prose, each a page ending in `>`.
3. **Objective block** — `Theater of operations:` / `Enemy forces:` / `Conditions
   for success:` (48 of these), ending in `/`.
TIM thumbnails for the mission are interleaved among these entries.

## Text encoding (same as `draw_string`, `REFERENCE.md` §4)
- **Terminators:** `>` (0x3E) ends a page; `/` (0x2F) ends the final
  objective screen (only treated as terminator before padding/space).
- **Line breaks:** literal `CR` (0x0D) / `LF` (0x0A) in-band.
- **Colour markup:** literal ASCII `@d` (palette/highlight on) and `%d`
  (column/format) sequences, e.g. `@1Guards@0`, `%2Enemy forces:`. `@`+digit
  matches the documented `draw_string` `@`-escape (`REFERENCE.md` §4); `%`+digit
  is a MIS-only formatter (exact semantics TBD — Phase F).
- **100% ASCII** → fully renderable by the stock renderer, so the existing Farsi
  `draw_string` path (`string_render.c`) renders translated MIS text unchanged.

## Translation notes
- ~**65k chars** of mission text (names + briefings + objectives).
- Editing is **in-place per entry** but each `.T` entry carries a trailing
  checksum word (`REFERENCE.md` §6) — recompute on re-inject or the loader hangs.
- Entries are 1 sector (2048 B) each; translated text must fit the entry's sector
  span (RTL Farsi is typically shorter than English, so headroom is fine).
- Mission number → name slot → briefing block is a direct structural map; no code
  table needed to localize (see `docs/DISC_TEXT_INVENTORY.md`).
