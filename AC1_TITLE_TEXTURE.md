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

## SESSION FINDINGS (2026-06-08) — source located, tampering bug found, approach decided

**1. Source located (no separate texture).** The 6 titles are pre-baked **word-art
in the menu FONT SHEET**, not a standalone TIM. It is `MS/MENU_TIM.T` **entry 0**,
embedded TIM @ file off `0x2E20` (image data `0x2E60`), **256×192 4bpp**, VRAM
`(448,0)` = **tpage 0x0007**, CLUT **0x3817**. The 6 words live in **rows V120–191**
in a 2-col × 3-row grid (~24px/row):
  - V120–143  MISSION | MAIL
  - V144–167  GARAGE  | RANKING
  - V168–191  SHOP    | SYSTEM
  Approx visual rects in `docs/title_wordart_rects.json` (exact title-SPRT UVs still
  to be confirmed by RE'ing the banner draw — that draw lives in a runtime overlay,
  not in the Ghidra base-EXE image, so it needs live overlay disassembly).

**2. TAMPERING BUG CONFIRMED (this is why titles are garbled — it is OUR doing).**
  - The working base `…/AC_1_USA_test/Armored Core (v1.1).bin` has MENU_TIM font
    rows **V96–188 overwritten with our 128-glyph Farsi atlas** (+ Persian digit
    cells), which **destroyed the English word-art**. It is baked into the base
    `.bin`, so EVERY `[RTL]` build inherits the garbled titles.
  - Proven by re-extracting MENU_TIM from the PRISTINE backup
    `…/AC_1_USA_backup/Armored Core (v1.1).bin` and decoding the font sheet:
    intact MISSION/MAIL/GARAGE/RANKING/SHOP/SYSTEM word-art. See
    `docs/screens/menu_font_PRISTINE.png` vs `menu_font_RTL_atlas.png`.
  - FDAT.T on the [RTL] disc is byte-identical to retail EXCEPT entry-201 (our
    draw_string + name-shaper code). So no *other* tampering — only this and the
    intended overlay patch.

**3. CORE CONFLICT.** `draw_farsi` (string_render.c, via `fmet[]`) reads name-screen
Farsi glyphs from **V97–188** of this same resident sheet — i.e. exactly the title
band. One sheet cannot hold both the glyph atlas and title word-art bitmaps.

**4. APPROACH DECIDED — Option A (composite titles from atlas glyphs).** Keep the
atlas where it is (name screen needs it); do NOT bake word-art bitmaps. Instead,
RE the title banner draw and redirect it to a `draw_string` of the shaped Farsi word
(گاراژ/رده‌بندی/نامه/سیستم/مأموریت/فروشگاه) rendered from the resident atlas, scaled
to read big. Under this approach the "destroyed word-art" is moot — we stop using
those rects as titles. The OLD bitmap plan below (steps 2–3) is SUPERSEDED.

## TITLE DRAW — FULLY RE'd (2026-06-08, live, slot 3 hub)
- **Title sprite:** one 32-byte textured-rect prim, code 0x64, drawn 1:1 at **screen
  (90,72) size 128×24** from **tpage 0x0007** (global/shared menu page). Prim built
  in the buffer at ~0x801D6948 (u/v at prim+0x18).
- **Title draw fn:** **`0x80065400`** (in FDAT entry-201 overlay → patchable). Loads
  descriptor ptr `s3` from working struct `0x801A37C8` field +0x0C (= `0x801A37D4`,
  holds current `0x800B6E70`); copies {u,v,clut,w,h} from the descriptor into the
  prim (the u/v store is at `0x800655AC`–B0). Calls base-exe helpers 0x8002CEF4,
  0x8002B8C4. **tpage is NOT in the prim/descriptor** — title uses the shared global
  tpage 7, so redirecting to another VRAM page needs a code patch (or a POLY_FT4).
- **TITLE DESCRIPTOR TABLE (the prize): `0x800B6E4C`, stride 12, 6 entries,
  {u:1, v:1, clut:2, w:2, h:2, flag:4}. CONFIRMED present & identical in
  fdat_extracted.T entry-201 at flat off 25,991,340 (0x18C98AC) → directly
  patchable.** Mapping (category id → word):
    id0 (0,144)=GARAGE  id1 (0,168)=SHOP   id2 (0,120)=MISSION
    id3 (128,168)=SYSTEM id4 (128,120)=MAIL id5 (128,144)=RANKING
  All clut=0x3817, w=128, h=24, flag=0x27.
- **Free VRAM:** large empty block ~x704–896, y128–255 (**tpage 11**, base x704). Room
  for six 128×24 word cells (2 cols u0/128 × 3 rows v128/152/176). tpage 7's own
  bottom rows (y192–255) are NOT free (see docs/screens).

## A2 plan (crisp titles, decided): render six **128×24** Farsi words at native size
(full PIL+raqm shaping, 4bpp, clut 0x3817 white), place in free VRAM, repoint the 6
descriptors, and set the title sprite's tpage. Open sub-problem: getting pixels into
free VRAM (no TIM targets it) — either embed a TIM + runtime LoadImage in the overlay,
or extend/relocate. Title draw fn 0x80065400 is patchable for the tpage set.

## LIVE PROOF — ALL 6 CRISP TITLES CONFIRMED (2026-06-08)
Rendered six 128×24 crisp Farsi words (tools/title_wordart.py), packed to 4bpp band,
and blitted live into the font sheet at V120–191 (DuckStation write_vram_region @VRAM
(448,120) 64×72) — i.e. exactly where the 6 descriptors already point, **tpage 7, no
code changes**. Rotated all 6 categories: سیستم·مأموریت·فروشگاه·گاراژ·رده‌بندی·نامه all
render crisp & correctly shaped (hamza + ZWNJ intact). Shots: docs/screens/title_crisp_*.png.
Proof save state = **DuckStation slot 4** (live VRAM hack, on MAIL). Clean hub = slot 3.
This validates rendering + descriptor map + draw RE completely.

## PERMANENT-BAKE PLAN (remaining)
The live hack clobbers the name atlas in VRAM (V120–191). For the disc, words must
COEXIST with the atlas → put words in **free VRAM tpage 11** and redirect the title:
- **Upload source:** entry-201 has NO ≥0x800 zero-run (scan: 0 free blocks) → cannot
  embed 9KB (or 2.3KB 1bpp) words in the overlay. Store word pixels in **MENU_TIM.T**
  (1.5 MB, room) as a new entry, read via read_T_entry into scratch RAM, LoadImage to
  tpage 11 (VRAM 704,128) via a one-time hook (guard flag) in the title draw fn
  0x80065400. (Alt: append to entry 0's buffer — but that shifts the whole TOC.)
- **Repoint:** patch the 6 descriptors @0x800B6E4C (FDAT entry-201) u,v → tpage-11 cells.
- **tpage:** patch title draw fn 0x80065400 to set the title sprite tpage=11 (only the
  title; don't disturb shared menu text). SPRT has no tpage field → emit a DR_TPAGE or
  convert to POLY_FT4.
- **Bake:** recompute entry-201 + MENU_TIM entry checksums, build [RTL] disc, verify.

## Earlier note: find the title banner draw site (DONE above via write-watchpoint on the
## title prim's u/v field — caught the descriptor-copy at 0x800655B0, fn 0x80065400).

## KEY SIMPLIFICATIONS (2026-06-08, second pass)
- **tpage IS in the descriptor.** The title fn calls SetDrawMode (base-exe 0x8002CEF4)
  with `tpage = descriptor+0x8` — i.e. the 4 bytes I called "flag=0x27" ARE the tpage.
  0x27 = tpage 7 (tx=7→x448, 4bpp, abr=1). So the 12-byte descriptor is really
  {u:1, v:1, clut:2, w:2, h:2, **tpage:4**}. ⇒ **Redirecting the title to free-VRAM
  tpage 11 needs NO code patch — just set the descriptor tpage field 0x27 → 0x2B**
  (tx=11→x704, keep abr=1). Title side is then 100% descriptor edits in FDAT entry-201.
- **Title screen position source:** title fn loads x = `lw 0x48(s2)`, y = `lw 0x4C(s2)`,
  s2 = 0x801A37C8 → live x @ **0x801A3810**, y @ **0x801A3814** (default 90,72). Adjusted
  live to (112,86) via freeze for centering — pending user sign-off. For the bake, find
  where these are written (hub menu init) and patch the constants. (Drawn 128 wide; my
  word cells are centre-anchored, so sprite-centre = x+64.)
- Net remaining work shrinks to: (1) upload words to tpage 11 (LoadImage hook + pixels in
  MENU_TIM.T), (2) descriptor edits (u,v + tpage 0x27→0x2B), (3) bake x/y position const,
  (4) checksums + disc build + verify.

## FINALIZED SPEC (user-approved 2026-06-08)
- Words (carousel id → Farsi): id0 GARAGE گاراژ, id1 SHOP فروشگاه, id2 MISSION مأموریت,
  id3 SYSTEM سیستم, **id4 MAIL ایمیل** (changed from نامه → e-mail), id5 RANKING رده‌بندی.
  All in tools/title_wordart.py (render 128×24, full raqm shaping). PROVEN crisp live.
- Title screen position: **x=112, y=86** (was 90,72) — approved centered above icon.
  Live source 0x801A3810/0x801A3814; for the bake find where these consts are written
  (hub menu init writes 90/72) and patch to 112/86.
- Upload-hook free space CONFIRMED: ~573B padding @ **0x800CF35F** (entry-201 tail before
  checksum @0x847FC). Enough for read_T_entry + LoadImage(RECT{704,128,128/4? ,72}) + flag.
  (Other small free runs: 0x800CEB58/128B, 0x800BC5B6/130B. AVOID 0x800BEB53 = boot table.)
## ✅ SHIPPED (2026-06-08) — crisp Farsi titles baked & cold-boot verified
The on-disc storage problem (100%-full disc, fragmented/unsafe overlay space) was
solved by a **zero-code "dead-kanji repurpose"**:
- The USA build still LOADS the Japanese kanji font sheet (**MENU_TIM.T entry 4**,
  256×256 4bpp @ VRAM **(576,256)**) but never RENDERS kanji → dead resident texture.
- `title_build.py`: overwrite entry-4's top 72 rows with the six 128×24 Farsi word
  cells (img @file 0x7F220), fix entry-4 checksum (@0x9F7FC), inject. Then write the
  6 retargeted descriptors @0x800B6E4C (tpage **0x39** = VRAM(576,256); /tmp/desc_kanji.bin)
  into FDAT entry-201, fix the entry-201 checksum, inject. **No overlay code, no upload
  hook, no MENU_TIM size change.** Operates on top of the existing [RTL] disc (preserves
  the name shaper). Backup: `[RTL].bin.pre-title.bak`.
- Cold-boot verified: all 6 titles render crisp (گاراژ·رده‌بندی·ایمیل·سیستم·مأموریت·فروشگاه),
  name atlas + Farsi menus intact, no NOW-LOADING hang. Shots docs/screens/baked_title_*.png.
  Baked-disc hub save state = DuckStation slot 6.
- Position left at the original (90,72) for v1 — user said tweak letter placement later
  (source x@0x801A3810 y@0x801A3814; centered look was 112,86).

## BUILD RECIPE (execute-ready, 2026-06-08) — historical; superseded by title_build.py above
Disc is 100% packed (no free sectors in MENU_TIM or FDAT) → store words IN the overlay
(entry-201), which we already rebuild+checksum (zero risk to other assets).
- **Words data:** 6 words pack to ~1167B @1bpp (tight). Reclaimable overlay space:
  dead `draw_kanji` 0x80065DBC..0x800660C4 (776B) + tail pad 0x800CF35F (573B) + small
  runs (188/130/128/85/76B). Pick a layout that (a) fits reclaimed space @1bpp and (b)
  keeps each word inside ONE 256-texel tpage column (don't straddle x768). Simplest:
  2 cols(≤128) × 3 rows in a 256×~40-48 block (needs ~1280-1536B @1bpp → use draw_kanji
  +tail, multi-segment unpack) OR a single 256-wide × 24 cap.
- **Upload hook (new code, in reclaimed overlay space):** guard-flagged once-only;
  unpack 1bpp→4bpp into scratch RAM (free: 0x801F50AE region, use ~0x801F5800), then
  `LoadImage(RECT{x=704,y=128,w=texels/4,h},scratch)` (**LoadImage @0x8002C2AC**),
  DrawSync(0). Trampoline: detour title fn first instr @0x80065400 (orig
  `lhu s4,0x14(s2)`=0x96540014) → `j hook`; hook runs upload-once, executes displaced
  instr, `j 0x80065404`. Hook must preserve s-regs (s2 used after).
- **Descriptors @0x800B6E4C (FDAT entry-201):** per-word u,v,w,h into the tpage-11 cells
  + tpage byte 0x27→**0x2B** (tx=11). Block proven live = /tmp/desc_tpage11.bin (for the
  256×72 cell layout; regenerate to match final packed layout).
- **Position const:** patch where 90,72 is written into 0x801A3810/0x801A3814 → 112,86.
  (Find writer: hub menu init. Live source confirmed; const location TBD.)
- **Checksums + build:** fix_overlay_checksum (entry-201, seed 0x12345678) → build_rtl_patch
  → boot → verify ×6 + name screen intact. NOTE entry-201 checksum MUST be recomputed.
- Proof states: slot 4 (tpage-7), slot 5 (tpage-11 final config). tools/title_wordart.py.

## (superseded earlier idea) Bake order: add words TIM entry to MENU_TIM.T (+fix entry checksum) → write upload hook
  @0x800CF360 + trampoline at title fn 0x80065400 → patch 6 descriptors @0x800B6E4C (u,v +
  tpage 0x27→0x2B) → patch x/y const → fix entry-201 checksum → build_rtl_patch → verify x6
  + confirm name screen intact.

## Plan (next session) — SUPERSEDED by Option A above; kept for reference
- DuckStation `load_state` **slot 2** = the DATA screen ([RTL] disc, save loaded
  from card, pilot سلام). From there: Circle → hub carousel; Left/Right rotate
  categories; the title banner is the garbled text at the top.
- Reach hub from cold boot: see `AC1_DATA_SCREEN.md` (demo-reel skip: 2 Start
  presses, 120-frame gap; tool `tools/ac1_mcp_input.py`).
- VRAM tooling: `dump_vram` (png/bin), `read_vram_region`. Today's dumps were in
  DuckStation's mcp cache (`vram_system.bin`, `vram_mission.bin`).
