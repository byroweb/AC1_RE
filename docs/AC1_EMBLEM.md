# AC1 player emblem — memory-card save format

Status (2026-06-13): **FULLY CONFIRMED** by reading a real DuckStation card that
contains drawn emblems. Offset + count + layout all resolved (see "Emblem format
— confirmed" below). Tooling round-trips, including a boundary-crossing slot
(handled in the companion AC1mod project — separate repo). The original
2026-06-09 guesses (one emblem in the MAIN save at 0x0E80, palette 0x0C3A) were
WRONG — the emblems are in a separate card file; that 0x0C3A ramp in the main
save is the live colour-picker state.

## Emblem format — confirmed
- Emblems live in a **separate card file**, code `BASCUS-94182Z` (note the `Z`
  suffix vs the main save's `A`), title **`ARMORED CORE EMBLEM DATA`**, **2 blocks
  / 16384 B**. The main save (`...A`) does NOT hold the drawn emblems.
- **7 emblems.** Each is a **2080-byte (`0x820`) record**, back-to-back:
  - `+0x00` 32 B palette = 16 × u16 BGR555 (a copy of the in-game colour picker
    rainbow ramp; in practice all 7 copies are the standard ramp)
  - `+0x20` 2048 B pixels = **64×64, 4bpp, low-nibble first, row-major**
- First record at **file offset `0x206`** (file = the save's blocks joined in
  link order). Emblem `i` record = `0x206 + i*0x820`; pixels = record `+0x20`.
- A single emblem's 2048-B pixel run **can straddle the 8192-B block boundary**
  (e.g. emblem 3 spans 0x1A86..0x2286), so edits must go through the joined file
  bytes, not one block. Default-blank pixels are `0xFF`.
- Ground truth: this card's 7 slots decode to hand-drawn `X,2,3,4,5,6,7` (each a
  different palette index) — upright and legible, confirming row + nibble order.
- This matches the user's earlier Project Phantasma (SLUS-00670) decoder exactly
  (same 7-count, same `0x820` stride, same interleaved per-emblem palette); only
  the base offset and the dedicated `...Z` file differ.

## Memory card (standard PS1, 128 KiB)
- 16 blocks × 8192 B. Block 0 = directory; frame 0 = `MC` header; frames 1..15 =
  one 128-B directory entry per data block:
  - `+0x00` u32 state (`0x51` first/in-use, `0x52` mid-link, `0x53` end-link, `0xA0` free)
  - `+0x04` u32 file size; `+0x08` u16 next-block link (`0xFFFF` none)
  - `+0x0A` filename ASCII (e.g. `BASCUS-94182A`); `+0x7F` u8 XOR checksum of `0x00..0x7E`
- Data block first frame: `SC` magic; `+0x02` icon flag (`0x11/12/13`=1/2/3 frames);
  `+0x04` title Shift-JIS (full-width); `+0x60` icon CLUT (16×u16 BGR555);
  `+0x80..` 16×16 4bpp icon frame(s).

The reference card (in your DuckStation memcards directory, e.g.
`~/.local/share/duckstation/memcards/<game>_N.mcd`) holds one AC1 save: slot 1, code `BASCUS-94182A`, title
`ARMOREDCORE01 SORTY000`. Its PS1 icon decodes to the **AC logo** (confirms the
card parser + 4bpp/BGR555 path).

AC1 detection: filename contains `SCUS-94182` / `SLUS-01323`, or title starts
`ARMOREDCORE`. Only AC1 saves are openable for emblem edit; others are out of scope.

The reference card above (`(Reprint)_1.mcd`) has only the main save and **no
emblem file**, so it shows nothing to decode — that earlier dead end was looking
in the main save. The card with drawn emblems is `Armored Core 1
backup_1.mcd` (slot 2 = `BASCUS-94182Z` `ARMORED CORE EMBLEM DATA`).

AC1 detection: filename contains `SCUS-94182` / `SLUS-01323`, or title starts
`ARMOREDCORE`. The emblem file additionally has `EMBLEM` in its title.

## How it was locked (2026-06-13)
No draw/byte-diff was needed: a card already had a fully-drawn emblem file. Export
both files (`export_memory_card_save`), join the emblem file's 2 blocks, and
scan for the colour-picker ramp signature (`1F 00 FF 01 FF 03`). It hits **7
times at stride `0x820`** starting `0x206` → 7 records, palette-then-pixels,
exactly. Decoding each record's 2048 B as 64×64 4bpp gives upright legible
glyphs. Done. (The former "main save @0x0E80" plan was a wrong file entirely.)

Former open questions, now answered: pixel offset = `0x206 + i*0x820 + 0x20`;
count = 7; they live in the `...Z` file, not the main save / not VRAM splits.

## Tooling (done, round-trip verified)
Implemented in the companion AC1mod project (separate repo):
- `core/memcard.py`: `read_card`, save list + AC1/emblem-file detect, `icon_rgba`,
  `file_bytes`/`write_file_bytes` (join/split the emblem file's 2 blocks),
  `emblem_file()`, and per-index `emblem_palette` / `decode_emblem` /
  `is_emblem_blank` / `encode_emblem` (GIF/PNG → nearest-colour in that emblem's
  palette → 64×64 4bpp) / `write_emblem`. `save()` recomputes the directory XOR
  checksum. Indices `0..6`; offsets via `emblem_record_off` / `emblem_pix_off`.
- a `memcard {list,icon,emblem-export,emblem-import}` CLI — `--card`,
  `--index N` (0..6; export -1 = 7-up sheet), `--image`, `-o`. Verified on a real
  card: list shows 7/7 drawn; export renders X,2,3,4,5,6,7; importing into the
  boundary-crossing slot 3 changes **only** that emblem's pixels, preserves all 7
  palettes + every other byte, and keeps directory checksums valid.
