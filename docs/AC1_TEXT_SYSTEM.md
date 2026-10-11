# AC1 (USA v1.1) — Text & Menu Rendering System

Reverse-engineering notes for the Armored Core 1 USA text pipeline (the **stock**
system). The Farsi-localization changes layered on top of it are documented in
the mod: [`mods/farsi/docs/LOCALIZATION.md`](../mods/farsi/docs/LOCALIZATION.md).

All RAM addresses below are for **FDAT entry 201**, the menu/UI overlay, which
loads at **RAM base `0x8004ADA0`** (segment `FDAT_201` in Ghidra). Entry-201
file offset → RAM: `RAM = 0x8004ADA0 + file_off`. The overlay carries a trailing
checksum word — see `project_ac1_overlay_checksum`; any byte edit to entry 201
must recompute it or the game hangs at NOW LOADING.

Behavioral C reconstructions of these routines live in `mods/farsi/`:
`font_render.c` (draw_char), `string_render.c` (draw_string; its 2-byte branch is
replaced by the mod's draw_farsi — read it as the stock routine plus one patch).

---

## 1. Font texture (MENU_TIM.T entry 0)

- 4bpp TIM, **256×192 px**, embedded inside MENU_TIM.T entry 0 at file off `0x2E20`.
- VRAM **(448,0)** → tpage **`0x0007`**. CLUTs: menu/white **`0x3817`** (VRAM 368,224),
  HUD/colour **`0x3C16`** (VRAM 352,240).
- Glyph grid: 32 columns × 8px. `U = ((code-0x20)&0x1F)*8`. V band depends on size.

V-band layout (THIS IS A USED-IN-FULL TEXTURE — no dead rows in the USA build):

| V range  | size mode | content                                            |
|----------|-----------|----------------------------------------------------|
| 0–47     | LARGE 8×16| ASCII `0x20–0x7F` (symbols / UPPER / lower)         |
| 48–71    | SMALL8 8×8| ASCII                                              |
| 72–95    | SMALL6 6×8| ASCII                                              |
| 96–119   | SMALL4 4×8| ASCII (tiny) — **still used**                       |
| 120–191  | —         | **pre-baked English word sprites** (NOT kana!):    |
|          |           | MISSION / MAIL / GARAGE / RANKING / SHOP / SYSTEM  |

The USA build repurposed the JP kana/kanji rows (V120–191) for English menu
word-art. Reference dump: `/tmp/ac1_font_real.png`.

---

## 2. Low-level glyph drawers

### draw_char @ `0x800660C4`  (see `font_render.c`)
Renders ONE ASCII char as a `SPRT_VAR` (GP0 0x64) into the OT.
- `col=(code-0x20)&0x1F`, `U=col*8`; V/size per size_mode (LARGE/SMALL8/6/4).
- Screen pos goes through a per-zone metrics transform:
  `style = zone_to_style[ctx->zone]; fm = 0x801BCE88 + style*100;`
  `prim->x = fm->adj_x + (x - fm->ref_x); prim->y = fm->adj_y + (y - fm->ref_y);`
  (variant stride 103512; bank 0 in menus). The Farsi path MUST replicate this.
- CLUT = menu (0x3817) when palette==0 else HUD (0x3C16).
- Appends a DR_MODE setting tpage 0x0007 after each glyph.

### draw_kanji @ `0x80065DBC`  (2-byte Shift-JIS path)
Originally invoked by draw_string for double-byte characters. **Was assumed dead
in the USA build — it is NOT; the name-entry keyboard depends on it** (§4, §5).
Its invocation was removed when patching draw_string (§6); this is what broke
name entry. Candidate space to reclaim for the runtime shaper.

### draw_string @ `0x8006599C`  (see `string_render.c`)
Walks a Shift-JIS string in `ctx->str`, emitting glyphs until `>` (0x3E).
- Single-byte ASCII `0x20–0x7F` → draw_char.
- ORIGINAL: lead bytes `0x81–0x9F` / `0xE0–0xFF` begin a 2-byte char → draw_kanji;
  `0xA0–0xDF` (katakana) drawn single-byte.
- Inline escapes: `\r`+skip (CR), `;` (newline), space, `{`/`}` digit (x-=/x+=),
  `^`digit (size mode), `~` (y-=4), `@`digit (palette).
- FontCtx fields used: `+0x08 zone`, `+0x0C str`, `+0x14 z_depth`, `+0x16 render_mode`,
  `+0x18 ot[12]`, `+0x48 x`, `+0x4C y`.

---

## 3. The menu draw-command queue

The UI doesn't call the drawers directly; it builds an array of **draw-command
structs** and a processor renders them once per frame.

### Processor @ `0x8009C1FC` (inside loop starting ~`0x8009C07C`)
For each command `a3`, it dispatches on the **type field at struct `+0x0A`**:

| type (`+0x0A`) | renderer       | notes                                  |
|----------------|----------------|----------------------------------------|
| 1              | `0x80066454`   |                                        |
| 2              | `0x80064874`   |                                        |
| 3              | `0x800653D4`   |                                        |
| 4              | `0x80057190`   | uses `ot[font_variant]` (`0x801EF6C8`) |
| 5              | `0x800656B8`   |                                        |
| 6              | `0x80059B50`   | **GTE/poly text renderer** (kana grid) |
| 7              | `0x8006599C`   | **draw_string**                        |
| 8              | `0x8005F5E8`   |                                        |
| else           | `0x80064EB0`   |                                        |

Common struct fields (offsets seen across constructors): `+0x0A type`,
`+0x0C str ptr`, `+0x10` (byte param), `+0x14`, `+0x16 render_mode (0x15)`,
`+0x18.. ot ptrs`, `+0x48 x`, `+0x4C y`, `+0x70/74/78` renderer-specific ptrs.

### Command constructors (entry-201 helpers)
- `h_d80c` @ `0x8005D80C` → **type 7** (draw_string). Used for labels
  (PILOT NAME / AC NAME ENTRY). Args `(a0,a1=str,a2,a3, sp+:x?,y?)`.
- `h_cb94` @ `0x8005CB94` → **type 6** (renderer `0x80059B50`).
- `h_d15c` @ `0x8005D15C` → **VRAM→VRAM `MoveImage` text blitter**
  (`src U=448+col*2, V=row*16` = LARGE font; `jal MoveImage 0x8002C374`). Pre-renders
  strings to VRAM scratch (e.g. x≈960). Reads LARGE band (intact).

---

## 4. Name-entry subsystem (pilot & AC names)

Reached from NEW GAME. It is the **Japanese kana input engine**, kept in the USA
build. The character map is built from **2-byte Shift-JIS** strings.

### Strings (entry-201)
- Labels: `PILOT NAME` @ `0x8004C6F4`, `AC NAME` @ `0x8004C6EC`,
  `AC NAME ENTRY` @ `0x8004CBA4`, `Confirm the registered name?` @ ~`0x8004C9*`.
- Grid rows (5 kana rows + 1 Latin row), pointed to by the 5-loop table
  **`0x800B85F4`**:
  - `0x8004C700` `おこそとのほも ろ ┼`
  - `0x8004C718` `えけせてねへめ れ ━`
  - `0x8004C730` `うくすつぬふむよるん`
  - `0x8004C748` `いきしちにひみゆりを〜`
  - `0x8004C760` `あかさたなはまやらわー`
  - `0x8004C854` `ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱ…` (full-width Latin/number/symbol)

### Render path
Grid + labels are queued draw-commands. The label path is draw_string (type 7);
the grid renders the 2-byte chars per cell. Each 2-byte char draws as one glyph
via the kanji path **in the stock game**.

### Cursor / state
- State machine jump table @ `0x8004C89C` (5 states; state 2 → `0x80081608`,
  others → `0x80081630`).
- Per-screen cursor struct (`a1`):
  `+25` = screen mode (`0x41`='A' ⇒ AC-name screen, else pilot),
  `+26` = current name length (0..8, wraps at 8),
  `+30` = cursor **column**, `+31` = cursor **row**.
- Input handler updates row/col with bounds checks (D-pad), wraps rows/cols.

### ACTIVE grid + append (USA = Latin/Greek, NOT kana)
The real keyboard is the **8-row Latin/Greek grid**, table **`0x800B8608`** (drawn
by the 8-row loop @`0x8008151C`). Rows (full-width 2-byte SJIS):
```
[0] ＡＢＣ…Ｑ @0x8004C854   [1] ＲＳ…Ｚ @0x8004C840
[2] ａｂｃ…ｑ @0x8004C81C   [3] ｒｓ…ｚ @0x8004C808
[4] １２３４５６７８９０　　・．（）＋ @0x8004C7E4
[5] ─│┌┐┘└├┬┤┴　　−／∞＃＆ @0x8004C7C0  (box-char CODES, font draws them as Roman numerals)
[6] ΑΒ…Ρ @0x8004C79C        [7] ΣΤ…Ω　…━┼ @0x8004C778  (━┼ = SPC/END cells)
```
The **active append** is @`0x80082288` (reads `0x800B8608`, `col*2` stride, copies
2 bytes + '>'); dest buffers **PILOT `0x80031BE6` / AC `0x80031BD4`** (=PILOT−18,
selected by mode `25(s0)==0x41`); len at `26(s0)` wraps at 8; row-special writes
full-width space `0x81 0x40` when field `28(s0)` hi == `0x070F`.
The `0x800B85F4`/`0x80081950` (kana) path is the **unused JP variant**.

**Display path = per-byte draw_string** (proven by the VRAM screenshot: each
2-byte char drew as two glyphs, lead `0x82`→Farsi#2). Therefore **1-byte Farsi
isol-code rows will display correctly** (one glyph per cell) with NO display-code
change — only the append `col*2→col*1` + reshape, the grid row data, and cursor
col bounds need patching.

### (JP-variant) append routine @ `0x80081950`
On confirm (same shape, reads kana table `0x800B85F4`):
```
row = cur[+31];  col = cur[+30]
rowstr = ((u8**)0x800B85F4)[row]          # grid row string
src    = rowstr + col*2                    # 2-BYTE STRIDE
nameptr= name_buf + len*2                  # dest
nameptr[0] = src[0];  nameptr[1] = src[1]  # copy BOTH bytes
nameptr[2] = '>'                           # 0x3E terminator
len = (len+1) & wrap(8)
```
Row==3 special-cases the full-width space `0x81 0x40`.

### Name buffers (MAIN exe RAM, not the overlay)
- **AC name @ `0x80031BD4`**, **PILOT name @ `0x80031BE6`** (≈8 chars × 2 bytes +
  terminator). These feed save data, the confirm screen, and rankings.

### Name display
The stored bytes are rendered through draw_string (type 7). So whatever encoding
is stored, the **display will follow draw_string's interpretation**.

---

## 5. Farsi localization changes

Moved to the localization mod: [`mods/farsi/docs/LOCALIZATION.md`](../mods/farsi/docs/LOCALIZATION.md)
(draw_string Farsi branch, font-texture plan, name-keyboard rework, runtime
shaper). This file now covers only the **stock** text system.

---

## 6. Open questions / TODO pointers
- Where exactly renderer `0x80059B50` (type 6) samples its glyphs (GTE poly path)
  — only matters if any type-6 text is kept; name grid will move to draw_string.
- Free-VRAM decision if the font tpage is ever no longer shared (currently moot since
  full-Farsi reuses the font texture wholesale).
- Save-data impact of 1-byte name encoding (acceptable for a translation).
