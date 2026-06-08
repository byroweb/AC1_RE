# AC1 Ravens' Nest category-title word-art → Farsi (handoff)

Task: replace the six hub carousel **category title** word-art images with **big
Farsi** equivalents. English words, in carousel order (navigating Right):
**GARAGE · RANKING · MAIL · SYSTEM · MISSION · SHOP**.

Confirmed Farsi (user-approved 2026-06-08):
| EN | FA |
|----|----|
| GARAGE  | گاراژ |
| RANKING | رده‌بندی |
| MAIL    | نامه |
| SYSTEM  | سیستم |
| MISSION | مأموریت |
| SHOP    | فروشگاه |
(Word-art is rendered with PIL+raqm full Unicode shaping — no font-glyph-set
limits, so hamza etc. are fine.)

## Diagnosis (done this session)
- The titles are currently **corrupted/garbled** (see `docs/screens/` once added;
  zoomed grab shown to user). Garbled since before any of today's pokes — NOT
  caused by the live label experiments.
- **VRAM diff** between two categories (SYSTEM vs the next) showed changes only in
  the framebuffer (x<320); **nothing re-uploaded**. ⇒ all six titles draw from
  **one static resident texture** ("the texture everything is calling"), each word
  a different UV rect. They are NOT per-category uploads.
- That texture sits in the **menu/font texture page tpage 0x0007 (VRAM ~448,0)** —
  the SAME page our Farsi glyph atlas uses (string_render.c: tpage 0x0007, CLUT
  `FONT_CBA_MENU 0x3817` → CLUT VRAM (368,224)). Strong hypothesis: our Farsi
  atlas baking overwrote part of the title word-art. Confirm by comparing the
  title region on the **original** unpatched disc vs the `[RTL]` disc.

## NOTE: this is the TITLE texture, a SEPARATE problem from the menu FONT
The small menu labels (SAVE DATA, CREDITS, RANKING, MISSION REPORT, SLOT 1/2,
list headers, confirm prompts) are drawn by a **shared ASCII font renderer**
@`0x80066480` (glyph = `char-0x20`, fixed 8px grid) that **cannot** render the
variable-width Farsi atlas — only `draw_string` (the `>`-terminated path, a C
reimpl) can. Localizing those needs a renderer patch and is OUT OF SCOPE here.
(The `>`-terminated DATA rows Sorties/Success/Failure/Overall DO localize via
draw_string — verified live, bytes computed in `tools/shape_data_labels.py`, not
yet baked.) See that file + this session's notes if revisiting the font path.

## Plan (next session)
1. **Locate source.** Find the FDAT entry holding the title texture (candidate:
   `MS\MENU_TIM.T`; base-exe string table @0x80011018) and the per-word UV rects.
   Easiest: RE the title sprite draw (find the SPRT for the banner; read its
   u/v/w/h/tpage/clut), or breakpoint the title draw. Cross-check the VRAM page
   at (448,0). Determine pixel format (likely 4bpp + CLUT).
2. **Generate word-art.** Render each Farsi word to a bitmap sized to its rect,
   quantize to the texture's CLUT/4bpp. Reuse the PIL+raqm setup from
   `farsi_texture.py` (NotoSansArabic). New helper, e.g. `tools/title_wordart.py`.
3. **Inject + bake.** Write the word bitmaps into the source TIM in
   `fdat_extracted.T`, rebuild the `[RTL]` disc, recompute the **entry checksum**
   (seed 0x12345678 + word sum — see project_ac1_overlay_checksum /
   patch_draw_string.py `fix_overlay_checksum`), boot and verify each category
   title renders the Farsi word. Commit to AC1_RE.

## Resume aids
- DuckStation `load_state` **slot 2** = the DATA screen ([RTL] disc, save loaded
  from card, pilot سلام). From there: Circle → hub carousel; Left/Right rotate
  categories; the title banner is the garbled text at the top.
- Reach hub from cold boot: see `AC1_DATA_SCREEN.md` (demo-reel skip: 2 Start
  presses, 120-frame gap; tool `tools/ac1_mcp_input.py`).
- VRAM tooling: `dump_vram` (png/bin), `read_vram_region`. Today's dumps were in
  DuckStation's mcp cache (`vram_system.bin`, `vram_mission.bin`).
