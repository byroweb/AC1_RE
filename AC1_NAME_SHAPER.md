# AC1 Farsi pilot-name shaper — runtime contextual shaping in name entry

Status: **DONE & validated on the baked disc** (2026-06-07). Typing Persian on the
pilot/AC name screen now joins contextually (init/medi/fina forms + lam-alef
ligature) and reads right-to-left, persisted into the existing name buffer as
shaped draw-order glyph bytes. Boots clean (no NOW-LOADING hang), renders, types.

## Data flow

```
keypress (Cross) ─▶ name_input_handler @0x80082190 (overlay)
                      ├─ tok = SELTAB[row*17 + col]        (ordinal token)
                      ├─ LOGBUF[TYPED_LEN++] = tok          (logical scratch)
                      ├─ shape_name(LOGBUF, TYPED_LEN, NAME)  jal 0x800BC740
                      │     └─ contextual forms + lam-alef, RTL draw order, '>' term
                      └─ cur[0x1a] = shaped glyph count n    (=0x801A28EE)
cursor.update @0x80083A28 reads n ─▶ name_X = 136-n*16, cursor_X = 120-n*16
```

## Hook (already baked into fdat_extracted.T from a prior session)

- Trampoline `@0x80082168`: loads `a2 = *(0x801A2568)` (buttons), `a0 = cur`,
  `a1 = parent`, `jal 0x80082190` (handler), then `j 0x800823F4` (outer epilogue).
  Handler `ra = 0x80082180`.
- Live-verified on the name screen: `cur = 0x801A28D4`, `parent = 0x801A2858`.

## The count field (the one open question from planning — RESOLVED)

`cur[0x1a]` == `0x801A28EE` is the count the **baked RTL cursor patch**
(`0x80083A28`) reads to right-justify the name:
`cursor_X (0x801A7360) = 120 - n*16`, `name_X (0x801A7500) = 136 - n*16`
(verified live: n=1 → 104 / 120). The handler therefore writes the **shaped**
glyph count there (differs from the typed-key count when lam-alef collapses 2→1).
The typed-key count is kept separately in `TYPED_LEN` (scratch `0x8004C74C`) and
resynced to 0 whenever `cur[0x1a]==0` (engine clears it on entry / full backspace).

## Placement (all in VERIFIED-free overlay padding)

| Piece              | Addr        | Size  | Block / limit |
|--------------------|-------------|-------|---------------|
| `shape_name` code  | 0x800BC740  | 400 B | 436 B padding block (ends 0x800BC8EA) |
| `shape_name` tables| 0x80081F70  | 96 B  | 200 B padding block (ends 0x80082038) |
| `name_input_handler`| 0x80082190 | ≤432 B| **must end ≤ 0x80082340** (row-0 keyboard glyph string) |
| ordinal `SELTAB`   | 0x8004C700  | 68 B  | overwrites old glyph-byte table |

`shape_name`'s code and const tables are at *different* addresses (split linker
script); the code references the tables via absolute `lui/addiu`, resolved at link.
Because 0x800BC740 is 16-aligned the entry has no leading pad; callers bind to it.

### WARNING — `0x80065DBC` is NOT reclaimable
The plan assumed `draw_kanji @0x80065DBC` (776 B) was dead. It is **not**: that
region is the packed `fmet[]` font-metrics table read every frame by
`draw_string`/`draw_farsi`. Overwriting it garbles all on-screen glyphs (the font
atlas bleeds through). Caught by a live inject test; relocated to the padding
blocks above. Keep `0x80065DBC..0x800660C4` untouched.

## Ordinal SELTAB layout (4 rows × 17 cols)

Tokens: `0..32` letter ordinals (`farsi_runtime_shape.KEYBOARD` order: 0=alef …
32=alef-madda), `33` space, `34..43` Persian digits ۰..۹, `0xFC` END, `0xFF` blank.
Derived by translating the live glyph SELTAB letter-by-letter to its ordinal.

```
row0: 10 0f 0e 0d 0c 0b 0a 09 08 07 06 05 04 03 02 01 00   ص..ا
row1: ff 20 1f 1e 1d 1c 1b 1a 19 18 17 16 15 14 13 12 11   blank, آ..ض
row2: ff*7  22 23 24 25 26 27 28 29 2a 2b                   digits ۰..۹ (col7..16)
row3: 21 21 fc fc ff*13                                     space×2, END×2
```
Fixes a latent bug: the old glyph-byte SELTAB stored digits ۶/۹ as `0xFC`/`0xFF`,
colliding with the handler's END/blank sentinels. Ordinal tokens avoid that.

## Build

`build_rtl_patch.py` compiles `farsi_name_shape.c` (split code/tables) and
`farsi_name_input.c` (bound to `shape_name=0x800BC740`, `draw_confirm=0x8005D8D4`)
with `mipsel-linux-gnu-gcc -march=r3000 -mips1 -mfp32 -Os -falign-functions=4 …`,
merges the 4 patches, recomputes the entry-201 checksum (seed 0x12345678 + word
sum), and reinserts FDAT sectors → `Armored Core (v1.1) [RTL].bin`.
Size guards assert the handler ends ≤ 0x80082340 and the shaper pieces fit.

## Validation (2026-06-07)

1. `shape_name` direct-call on console (سلام ords → `E0 F5 AD`, n=3, lam-alef).
2. Deterministic handler call (cursor (0,0)=ص → LOGBUF=0x10, name=`B4 3E`).
3. Live keypresses: typed ص×3 → `B7 B6 B5` (fina-medi-init); typed سلام →
   LOGBUF `0e 1a 00 1b`, name `E0 F5 AD 3E`, rendered joined in the box.
4. **Baked disc, fresh boot**: Scenario → بازی جدید → name screen (no hang),
   typed سلام → "سلام" rendered with lam-alef ligature; Circle backspace reshapes.

## Default cursor position — FIXED (baked + validated)

The engine parked the cursor off-grid at entry: `col=16, row=7`
(`0x801A28F2`/`0x801A28F3`), a cell outside the 4×17 SELTAB → selecting it yielded
garbage. The selection-state init lives at `0x80081144`:
`addiu v0,zero,16; sb v0,0x9a(s3)` (col) then `addiu v0,zero,7` `@0x8008114C`;
`sb v0,0x9b(s3)` `@0x80081160` writes row to `0x801A28F3` (s3=0x801A2858, the
parent/state struct). Found via a write-watch on `0x801A28F3` during a fresh
name-screen build (caught at PC 0x80081164).

Fix = 1-byte patch on the addiu immediate: `0x8008114C: 07 → 00`
(`addiu v0,zero,7` → `addiu v0,zero,0`), so the cursor starts on **ALEF**
(col 16, row 0 = SELTAB[16]). Verified on a fresh boot: `0x801A28F2 == 0x0010`
(col16,row0), highlight on ا, and Cross there types `80 3E` (alef isolated) — no
more parking-cell garbage. In `build_rtl_patch.py` PATCHES as `0x8008114C: 00`.

## Proportional right-justify — FIXED (baked + validated)

The baked RTL cursor patch reserved a FIXED 16 px/glyph (`name_X = 136 − n·16`),
but `draw_string` renders Farsi name glyphs PROPORTIONALLY (`x += fmet[gidx].adv`,
advances 3..14 px; alef is only 3 px). So the rendered string was far narrower
than reserved and DRIFTED LEFT as it grew. `fmet` is at runtime `0x80065DA0`
(6-byte entries `u,v,w,h,dydx,adv`; adv at +5).

Fix: `farsi_name_xpos.s` (`name_xpos`) sums the actual `fmet` advances over the
`>`-terminated name buffer and anchors the right edge at x=136:
`name_X = 136 − Σadv`, `cursor_X = name_X − 16`, then tail-jumps to `cursext`
(0x80082130) for the per-frame label fix. cursor.update `0x80083A28` is repointed
to `j name_xpos`. Verified on a fresh boot: 2 alefs → name_X=130, 4 alefs → 124,
سلام → 110 — right edge pinned at **136** in every case; سلام renders tight and
flush at the box's right border.

Hand-assembled (96 B): GCC `-Os` emitted ~192 B which did not fit. Placed at
**0x80081FD0**, the proven-free tail of the 200 B padding block that already holds
the shaper tables (0x80081F70).

### WARNING — the 261 B "zero block" at 0x800BEB53 is a BOOT TABLE, not free
First attempt put `name_xpos` at 0x800BEB60 (a 261 B zero run). The disc HUNG at
boot: `internal_frame_number` frozen while `frame_number` advanced, PC in garbage,
the stack flooded with `name_xpos`'s own code bytes — boot code reads that region
as a (zero-initialised) data table and copied it. Zero-runs are NOT necessarily
free code space. Only regions proven by a clean boot (here: the shaper blocks
0x800BC740 and 0x80081F70) are safe; verify any new placement by booting.

## TODO

- Trace the confirm→memory-card save copy (write-wp `0x80031BE6`) to prove the
  shaped bytes round-trip on reload.
