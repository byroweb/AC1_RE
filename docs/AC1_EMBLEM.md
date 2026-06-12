# AC1 player emblem — memory-card save format (static RE + DuckStation plan)

Status (2026-06-09): emblem **palette + format confirmed statically**; exact pixel
**offset + count** pending a DuckStation byte-diff. Tooling already round-trips
(in the companion AC1mod viewer (separate repo): its `core/memcard.py` and `memcard` CLI).

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

## Emblem — what's confirmed
- **64×64, 4bpp (16 colours)**, RAW pixels (NOT an embedded TIM — the `0x10000000`
  hits in the block are false positives inside stats data).
- **Palette = the in-game colour picker**: 16 BGR555 entries at block offset
  **`0x0C3A`** — a clean rainbow ramp (red→orange→yellow→green→cyan→blue→magenta→
  gray→white). Player-editable, stored per save (so it is NOT a constant in
  `SLUS_013.23` — a binary search for the ramp finds nothing, as expected).
- One 64×64 4bpp emblem = **2048 B**, which fits cleanly in the block's blank tail
  region **`0x0E80..0x2000`** (4480 B, all `0xFF` in this save).

## Why this save shows "nothing"
This pilot never drew an emblem, so the pixel region is blank (`0xFF` = white).
The structured bytes at `0x0C00..0x0E77` are save **metadata** (a descending value
table at `0x0D00`, coordinate/RLE-looking pairs at `0x0D90`, an `00 FF` run that
renders as alternating red/white) — not the emblem raster. Decoding 64×64 from
`0x0C5A` shows metadata in the top ~17 rows then blank, confirming the emblem
proper is the `0xFF` tail.

## Open — needs DuckStation ground truth
1. **Exact pixel offset** within `0x0E80..0x2000` (default guess `0x0E80`; could be
   block-end-aligned `0x1800`).
2. **Count.** Earlier impression was "7 emblem textures". 7×2048 = 14336 B > one
   8192 block, so all 7 cannot be full 64×64 in the main save — likely the main
   save holds ONE 64×64 emblem and the "7" are either VRAM split-textures (the AC
   decal applied at several places) or live in a separate **SAVE EMBLEM** card file
   (the DATA screen has `SAVE EMBLEM`/`LOAD EMBLEM`). This card has no emblem file.

### DuckStation actions that would lock it (byte-diff, most decisive)
1. Boot AC1, load this save, go DATA → enter the emblem editor and **draw a
   distinctive pattern** (e.g. a diagonal of pure colour 0, then colour 8) so the
   indices are unmistakable. Exit/save back to the **same card**.
2. `export_memory_card_save` (or copy the `.mcd`) **before and after**, then diff:
   the companion viewer's memcard CLI reads both; the changed byte span = the emblem pixel region →
   exact `EMBLEM_PIX_OFF` and length (→ count). The drawn indices vs. our nibble
   order confirm bit/nibble packing and row order.
3. While in the editor, `dump_vram` / `read_vram_region`: the emblem is uploaded
   via `LoadImage` (string present at `0x80011BD0`). The VRAM RECT gives w/h in
   16-bit units (64px 4bpp → w=16, h=64) and the texture-page location, confirming
   4bpp and revealing how many copies/sizes exist (the "7" question).
4. Optional: breakpoint `LoadImage` (or the memcard read path) to capture the
   source pointer = the in-RAM save buffer + emblem offset, cross-checking #2.

Static RE of the loader itself needs the **DATA-screen overlay** imported into
Ghidra (the EXE currently loaded is the base + garage; its string table has no
`EMBLEM`). The byte-diff above is faster and decisive, so do that first.

## Tooling (done, round-trip verified)
Implemented in the companion AC1mod viewer (separate repo):
- `core/memcard.py`: `read_card`, save list + AC1 detect, `icon_rgba`,
  `emblem_palette`, `decode_emblem`, `encode_emblem` (matches any GIF/PNG to the
  fixed 16-colour palette, nearest-colour, packs 64×64 4bpp), `patch`/`save`
  (recomputes the directory XOR checksum). All offsets are parameters.
- `memcard {list,icon,emblem-export,emblem-import}` CLI — `--card`,
  `--slot`, `--image`, `--pix-off`, `-o`. Verified: import a PNG → export → blank
  flips to drawn, bytes byte-identical after reload, save stays a valid AC1 file.
