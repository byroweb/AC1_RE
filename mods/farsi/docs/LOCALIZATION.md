# AC1 Farsi localization — engine changes (draw_string, font, name keyboard, shaper)

Split out of the RE write-up [`docs/AC1_TEXT_SYSTEM.md`](../../../docs/AC1_TEXT_SYSTEM.md)
(§1–4 there describe the *stock* text & menu system; everything below is the
Farsi patch set layered on top of it). Section numbers are kept (5.x) so older
notes and commit messages that cite "§5.3" still resolve.

Source files: [`string_render.c`](../string_render.c) (draw_string + draw_farsi),
[`font_render.c`](../font_render.c) (draw_char), `farsi_*.py` (build-time pipeline),
[`farsi_runtime_shape.py`](../farsi_runtime_shape.py) (runtime shaper reference),
[`farsi_name_*.c`](../) (name-entry handler + shaper, R3000).

---

## 5. Farsi localization changes

### 5.1 draw_string Farsi patch (DONE — `string_render.c`)
The 2-byte kanji branch was replaced with a Farsi branch:
> any byte `>= 0x80` → Farsi glyph, index `= byte-0x80`, drawn by **draw_farsi**
> (variable-width SPRT_VAR from `fmet[]`, same per-zone metrics transform).

Glyph contract (`farsi_glyphs.py`): 128 glyphs, bytes `0x80–0xFF`. Each letter's
contextual forms are **contiguous**: D = `[isol,init,medi,fina]`, R = `[isol,fina]`.
Digits `d0..d9` = `0xF6..0xFF`; `lamalef.isol/.fina` = `0xF4/0xF5`.

**Side effects of removing the 2-byte path:**
- **Name entry broke** (confirmed via VRAM viewer 2026-06-03): the grid's 2-byte
  chars now render as TWO wrong single-byte glyphs each (e.g. Ａ=`82 60` →
  Farsi#2 + ASCII `` ` ``). The whole map is garbage.
- Any other stock screen using 2-byte text is similarly affected.

### 5.2 Font texture (planned — no-Roman full-Farsi build)
The user chose a full localization: drop Roman letters, make every cell Farsi.
- Keep the variable-width Farsi atlas for draw_farsi prose & names.
- **Persian digits ۰–۹ into the ASCII digit cells `0x30–0x39`** so the stock
  number rendering (credits/AP/stats/rankings) shows Persian numerals unchanged.
  DONE (`build_pdigits.py`): digit cells = cols 16–25, bands LARGE V0 / SMALL8 V48
  / SMALL6 V72 (SMALL4 V96 is Farsi atlas now). Output `/tmp/MENU_TIM_pdigits.T`
  (same 1,517,568 B), injected into the test BIN as `GG/MS/MENU_TIM.T`.
  Font = NotoSansArabic-Bold, 1-bit thr 90.
  **GOTCHA (hit it): MENU_TIM.T entry 0 HAS a trailing checksum** — entry 0 =
  sectors 1..34, base 0x800, word @0x117FC = 0x12345678 + Σ words[0x800..0x117F8].
  Editing the font without recomputing → hang at NOW LOADING. `build_pdigits.py`
  now runs `fix_entry0_checksum()` automatically. PENDING in-emulator verification.
- Replace the V120–191 word sprites (MISSION/…/SYSTEM) with Farsi word-art at the
  same VRAM cells.
- Earlier "overwrite V96–191 only" plan was wrong (that region is in use); see
  `project_ac1_font_vram_conflict`.

### 5.3 Name-keyboard rework (planned)

Stock grid is **17 cols × 8 rows** of 2-byte full-width chars (rows stored as
SJIS strings, e.g. row1 `ＡＢＣ…Ｑ`@`0x8004C854`, row2 `ＲＳ…Ｚ`@`0x8004C840`,
then lowercase / digits+punct / roman-numerals+punct / Greek×2, with SPC/END
bottom-right). The earlier "kana" strings (`0x8004C700`+) are leftover JP data.

**Finalized Farsi layout (user-approved 2026-06-04):** keyboard shows **isolated
forms only**; shaping is applied only to the typed name. Alef top-RIGHT, proceed
LEFT, wrap after 17 (RTL mirror of Latin). On-screen (left→right):
```
Row1:  ص ش س ژ ز ر ذ د خ ح چ ج ث ت پ ب ا     (letters ا..ص, 17)
Row2:  · · آ ی ه و ن م ل گ ک ق ف غ ع ظ ط ض    (letters ض..ی + آ, 16; 2 free far-left)
Row3:  ) ( . ـ ، ٫            ۰ ۱ ۲ ۳ ۴ ۵ ۶ ۷ ۸ ۹   (punct flipped far-left; Persian digits far-right)
Row4:  END  SPC                                    (controls bottom-left, mirrored)
```
- Digits = Persian ۰–۹ (atlas 0xF6–0xFF). Punct (left, flipped) = `٫ ، ـ . ) (`.
- Labels: PILOT NAME → `نام خلبان`, AC NAME → `نام ای‌سی`, NAME → `نام`.

**Encoding decision:** grid cells store the **isolated-form glyph byte**
(`0x80+isol_index`) for letters / the literal glyph byte for digits+punct, so the
keyboard DISPLAYS correctly via draw_string with no conversion. On select, the
append routine reverse-maps that byte → a **logical token** appended to a logical
scratch buffer:
- letters → ordinal 0..32 (shaper applies contextual form)
- digits / punct / ASCII → stored as the literal glyph byte (>=0x80 or <0x80),
  which the shaper passes through verbatim and treats as a **join break**.

**MONOSPACE keyboard (done 2026-06-04):** the grid renderer advances by each
glyph's own width (cumulative), so variable-width Farsi packed (~2 letters/cell).
Stock looked aligned only because full-width glyphs are a uniform 16px. Fix:
set the **isolated-form + digit advances to 16** directly in the fmet table
(flat `25,659,392`, adv = byte +5 of each 6-byte entry; isol indices + d0..d9 =
glyph 118..127). Connected forms keep natural advance, so prose is ~unaffected
(isolated forms are rare in connected text). Result: 1 glyph per 16px cell.
Digit cell baseline raised (build_pdigits BANDS dy +2 → −1) per "numbers too low".

**Cursor navigation — REWRITTEN (2026-06-04).** Stock nav (@`0x80081C5C`) was a
hand-tuned table for the irregular Latin layout (per-row 17/9/17/9 counts, gaps,
SPC/END) → caused row-skipping / landing on blanks with the Farsi grid. Replaced
the 4 direction-update blocks with **uniform clamped nav**, keeping each handler's
button-edge/repeat prologue intact:
- button bits: `0x8000`=LEFT (col−1 wrap16), `0x2000`=RIGHT (col+1 wrap0),
  `0x1000`=DOWN (row+1 wrap0@3), `0x4000`=UP (row−1 wrap3). col∈0..16, row∈0..3.
  (up/down/left/right mapping unverified — swap the 4 update blocks if reversed.)
- patched update words at `0x80081CF4` / `0x80081E1C` / `0x80081F48` / `0x800820C4`,
  NOP-filled the leftover stock code after each; common exit `0x80082168`.
- Cursor row/col live at `30(s0)`/`31(s0)`. All cells now render true **16px**:
  letters/digits adv=16; **blank cell = 2 spaces** (8+8px) for grid alignment.
- Rows: 0 = 17 letters (alef@col16), 1 = blank col0 + 16 letters, 2 = blanks +
  digits col7-16, 3 = "SPC END" (display only). MAXROW=3. ASCII punct dropped from
  the digit row (was 8px-misaligned; reinstate later as 16px atlas glyphs).
NOTE: SELECTION (append) still uses col*2 on this data — wrong until the text-entry
rework; nav/display only for now.

**GOTCHA: adv=16 breaks PROSE.** Setting isolated-form advances to 16 (for a
monospace keyboard) ALSO widens isolated forms in connected words — and prose uses
them constantly (alef/dal/re/vav are R = isol/fina; e.g. ادامه = alef.isol dal.isol
alef.isol mim.init he2.fina). Result: gaps in start-menu text. FIX: keep advances
NATURAL; make the keyboard monospace by PADDING each display cell to 16px with a
draw_string `}`/`{` escape ([glyph]['}'][0x30+(16-adv)], blank cell = 2 spaces),
and decouple SELECTION via a seltab (cursor-cell -> glyph) so display padding
doesn't affect typing. Rows relocated to 0x80082280+ (grid table 0x800B8608 repointed).

**GOTCHA: free space near entry-201 end is a trap.** The only big zero-run is right
before the trailing checksum word; a 544B function placed there RAN OFF the loaded
overlay -> crash. Verify offset+len <= E201_SIZE and < checksum word.

**GOTCHA: draw_kanji region is NOT free — it holds the fmet[] table.** The Farsi
draw_string expanded to 1828B = code + **fmet[128] metrics @0x80065DA0-0x800660C0**,
consuming the old draw_kanji space right up to draw_char(0x800660C4). Placing code
there clobbers fmet -> all Farsi glyphs break (garbled UV/adv). Restore fmet from
draw_string_full.bin[1028:1796] (+re-apply adv=16 on isol+digits). Only large free
run in entry201 = **0x800CF360 (573B)**; small free = the NOP'd nav blocks + handler tail.

**TEXT ENTRY — IMPLEMENTED (2026-06-04, compiled C, awaiting test).**
Replaced the stock confirm/append/backspace tail with a C handler:
- `shape_name` (farsi_name_shape.c) placed @**0x800CF360** (.text 544B in the big free
  run) with its .rodata tables @**0x80082290** (after the handler). [Earlier mistake:
  put it in draw_kanji and clobbered fmet — see GOTCHA above.]
- START (button 0x800) -> handler jumps cursor to END glyph (row3,col2). END token
  (0xFC) -> parent+116 = 0x8008240C (name-confirm state).
- `name_input_handler(cur, parent, buttons)` (farsi_name_input.c) placed
  @**0x80082190** (208B, in the old confirm/backspace region).
- Call site rewritten @0x80082168: `a2=*(0x801A2568)` buttons, `a0=s0` cur,
  `a1=s1` parent, `jal 0x80082190`, then `j 0x800823F4` epilogue; dead tail NOP'd.
- **SELTAB @0x8004C700** (stride 17, rows 0-3): cell -> logical token (letter
  ord 0..32, space 33, digit 34..43, END 0xFC, blank 0xFF). **LOGBUF @0x8004C744**
  (typed tokens; only [0..len) read so no init needed).
- Flow: confirm(0x40)-> tok=SELTAB[row*17+col]; blank ignore; END -> parent+116 =
  0x8008240C (confirm state); else LOGBUF[len++]=tok, shape_name(LOGBUF,len,name).
  backspace(0x20)-> len--, reshape. name buf = AC 0x80031BD4 / PILOT 0x80031BE6.
- Names CONNECT live (shaped each edit). RISKS to verify: (a) is 0x801A2568 edge
  vs held (held => rapid-fire; may need debounce); (b) END state 0x8008240C path;
  (c) name-box left-anchored RTL alignment. SPC at row3 col0-1, END col2-3.

**TEXT ENTRY v2 (2026-06-05, working): isolated letters, RTL, edge-detected.**
`name_input_handler` @0x80082190 (432B), rows @0x80082340+, seltab @0x8004C700,
PREVBTN @0x8004C748 (edge), LOGBUF @0x8004C750. Fixes applied:
- EDGE: edge = buttons & ~PREVBTN (one char per press, not frame-rate).
- DEBOUNCE for downstream states: handler sets `*(0x801A2550) |= edge&(0x40|0x20|0x800)`
  (the game's "consumed" latch) so the name-confirm state doesn't accept the held
  END press. Stock did the same; it had been dropped when switching to a custom edge.
- RTL: LOGBUF holds typed order; display = reversed(LOGBUF)+'>'.
- END: draw_confirm(0x8005D8D4, "Confirm name?" @0x8004C878, cur+24,12,0) + set
  parent+116=0x8008240C. START(0x800)->cursor row3 col2.
TODO: (a) name field is LEFT-justified, should RIGHT-justify (RTL) + input cursor
start at right moving left — needs the name-box draw code (find where name buf
0x80031BE6 is drawn + the cursor cell). (b) connected shaping (place shape_name
in a VALID free region, then route LOGBUF through it instead of plain reverse).

**(superseded) Append rework notes — TODO (the typing fix):** storing 1 byte/char (not 2) means
patching the dest math `name_base + len*2 → len*1` in BOTH branches around
`0x80082244`/`0x8008225C` (PILOT `0x80031BE6`, AC `0x80031BD4`), the col read
`0x800822A0` (`col*2 → col*1`), dropping the 2nd-byte copy `0x800822B4..DC`, and
the full-width-space special case (`0x070F` check → write `0x20` not `0x81 0x40`).
Also check the name-box cursor/position display for len*2. Then layer the shaper.

(old) append routine `0x80081950` (kana path) (stride `col*2→col*1`, reverse-map to
token, push to logical buf, then `shape_name()` → name buffer); cursor column
bounds for the new 17-wide layout; grid row data rewritten; name buffers
`0x80031BD4`/`0x80031BE6` now hold **shaped draw-order bytes** (display unchanged).
Punctuation glyphs (`٫ ، ـ`) are non-ASCII → need font cells (see §5.2 budget).

### 5.4 Runtime shaper (reference DONE — `farsi_runtime_shape.py`)
Typed names must connect. The shaper turns stored **logical-order letter
ordinals** into **draw-order shaped glyph bytes** for draw_string/draw_farsi.

On-console tables (one entry per keyboard letter):
- `LTAB[ord] = (isol_index, flags)`; `flags`: bit0 dual-joining(D), bit1 LAM,
  bit2 ALEF (alef / alef-madda — ligature target).

Per letter (single L→R pass, then reverse for RTL):
```
prev_conn = prev exists AND prev is D
next_conn = this is D    AND next exists
if this==LAM and next is ALEF: emit lamalef.fina if prev_conn else .isol; skip 2
elif D: off = [isol,init,fina,medi][prev_conn<<1 | next_conn]   # 0,1,3,2
else  : off = prev_conn                                         # R: isol/fina
emit 0x80 + isol_index + off
```
**Validation chain (all byte-identical):** build-time `farsi_shape.py` ==
`farsi_runtime_shape.py` (14 words incl. lam-alef + double ligatures
`بالالا`→`F4 F4 81 83`) == C `farsi_name_shape.c` (host `-DHOST_TEST`).

Handles **letters** (contextual form + lam-alef) AND **non-letter literals**:
a logical token `>= NLET(33)` indexes `LITERALS[]` (space, ۰–۹ = 0xF6–0xFF, then
`. ( ) ، ٫ ـ`), emitted verbatim and **breaking the join run** (a literal between
two letters stops them connecting — verified `با لا`→`F4 20 81 83`, the space lets
`لا` ligate). Neighbour checks guard on "is a letter" (`tok < NLET`).

**Build-ready (confirmed):** `farsi_name_shape.c` compiles for R3000
(`-march=r3000 -mips1 -mfp32 -G0 -O2 -ffreestanding -fno-builtin -fno-pic
-mno-abicalls`) to **.text 544 B + .rodata 96 B = 640 B, NO external function
relocs** (only HI16/LO16 into its own tables). Fits the reclaimed **draw_kanji
region 0x80065DBC–0x800660C4 (776 B)** with ~136 B margin.
TODO before reclaiming: confirm draw_kanji has no callers other than the (now
removed) draw_string branch.

C signature: `int shape_name(const u8 *toks, int n, u8 *out)` — toks = logical
tokens (letter ordinals 0..32, or literal indices >=33), out = draw-order bytes
+ '>' , returns count. Tables `LTAB_ISOL[33]`, `LTAB_FLAGS[33]` (bit0 D, bit1
LAM, bit2 ALEF), `DOFF[4]={0,1,3,2}`, `LITERALS[17]`.
Validated Python≡C≡build-time on 20 words (letters-only + mixed digit/space/punct).
