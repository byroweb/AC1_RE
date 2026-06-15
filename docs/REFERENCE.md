# AC1 (SLUS-01323) Reverse-Engineering Reference

A consolidated map of what's been reverse-engineered so far: functions, RAM
addresses, the `.T` container format, FDAT entry-201 (the text/menu overlay),
the name-entry engine, the patch pipeline, and disc sector locations.

All addresses are for the **North American release, `SLUS-01323` (v1.1)**.
They will **not** match other regions/revisions.

> This is factual interoperability documentation. It contains no game code,
> assets, or data — only addresses, offsets, and behavioral descriptions.

---

## 1. Target & memory map

- **Executable:** `SLUS-01323`, PS-X EXE, ~166 KB, MIPS R3000A little-endian.
- **Load base:** `0x80011000`  •  **Entry point:** `0x80011E6C`  •  **Text end:** `0x8003A000`
- First `0xE6C` bytes of the EXE are data tables (file-path table, debug
  strings, audio paths), not code.
- **BSS clear:** `0x80039CB8`–`0x8004ADA0`.

| Region | Range | Notes |
| --- | --- | --- |
| RAM (kernel) | `80000000`–`80010FFF` | BIOS / kernel |
| CODE (EXE) | `80011000`–`80039FFF` | the SLUS executable |
| RAM | `8003A000`–`801FFFFF` | heap, BSS, **runtime overlays** |

SDK: PSY-Q/libps late-1996 build (`bios.c v1.81 1996/12/16`). SN Systems CRT0
(startup symbol `start` / `__SN_ENTRY_POINT`). ~1056 Ghidra functions total
(~616 SDK-named, ~440 game-specific).

---

## 2. Boot & main loop

```
BIOS → start (0x80011E6C)
  → InitHeap(0x8004ADA4, …)
  → main (0x80011F28)
      → __main()                      C++ static constructors
      → wmemset / FUN_80015538        word-fill (custom memset, by 4 bytes)
      → CdInit()
      → InitPAD / _bu_init / StartPAD
      → FUN_80016F28(0)               memory-card init
      → FUN_800122A8()                GAME LOOP (infinite)
```

### Game loop — `FUN_800122A8`
```
ResetGraph / SetDispMask / SetFogNearFar / ClearImage
load_T_file(1..0x13, names)           load all .T assets

outer loop (forever):
  wmemset(workspace, 0, size)
  read_T_entry(FDAT, 0xC9, workspace)  initial FDAT state (entry 201)
  FUN_8009B414()                       OVERLAY — runtime-loaded code
  inner loop:
    switch DAT_80039C5E:                game mode flag (0/1/2)
      0 → read_T_entry(FDAT, 0xCA)
      1 → read_T_entry(FDAT, 0xCB)
      2 → read_T_entry(FDAT, 0xCC)
    (*DAT_8004ADA4)()                   FN POINTER — current state handler
    loop while result != 0
```
- `DAT_80039C5E` — game mode (0/1/2; title/menu/mission).
- `DAT_8004ADA4` — function pointer to the active state handler.

---

## 3. The `.T` container format (CONFIRMED)

Each `.T` file is a sector-based container:
- **Sector 0** (2048 B): TOC = packed `ushort` array of sector offsets.
  Entry *i* spans sectors `TOC[i]‥TOC[i+1]-1`.
  - **Caveat (MENU_TIM.T):** `ushort[0]` is the **entry count** (122), and real
    offsets start at `ushort[1]`. Don't treat `ushort[0]` as the first offset.
- **Sectors 1+**: entry payloads.

**Loaders / helpers:**
| Name | Address | Role |
| --- | --- | --- |
| `load_T_file` | `FUN_80016678` | read TOC header, register file in global table |
| `read_T_entry` | `FUN_800165E4` | read entry *i* from disc into RAM (verifies checksum) |
| `read_sectors` | `FUN_80016328` | raw CD sector read |
| seek/CdRead | `FUN_80015A08` | CD seek helper |
| `wmemset` | `FUN_80015538` | word-fill (ptr, value, word_count) |
| checksum verify | `FUN_80015B24` | see §6 |

**Global file-record table** at `0x8004A2A8`, 12-byte stride per file
(`+0x0` CdlLOC, `+0x4` size, `+0x8` reserved). TOC pointer for file *id* at
`0x8004A2A4 + id*12`. mxt_head_work allocator pointer `DAT_8004A394`.

### File IDs (from `load_T_file(id, path)`)
```
0  (stage PA*.T)  6  MS/MENU_VAB.T   11 MS/BST_T.T    16 MS/COMP_T.T
1  COM/RTIM.T     7  MS/CORE_T.T     12 MS/BWL_T.T    17 MS/SPEC_T.T
2  COM/FDAT.T     8  MS/LEG_T.T      13 MS/BWR_T.T    18 MS/GENE_T.T
4  MS/MENU_TIM.T  9  MS/ARMS_T.T     14 MS/WEL_T.T    19 MS/MIS.T
5  MS/MENU_TMD.T  10 MS/HEAD_T.T     15 MS/WER_T.T
```
The game loop (`FUN_800122a8`) registers ids 1,2,4–0x13 permanently. **The current
stage `PA##.T` is loaded into file slot 0** (`load_T_file(0, "Pn\PAnn.T")`) by the
mission overlay — *not* id 3 (earlier guess corrected by RE; see §11).

### FDAT.T entry indices
| Entry | Dec | Role |
| --- | --- | --- |
| `0xC9` | 201 | initial/common state setup — **the text/menu code overlay** |
| `0xCA` | 202 | game-mode-0 state handler |
| `0xCB` | 203 | game-mode-1 state handler |
| `0xCC` | 204 | game-mode-2 state handler |

FDAT.T has 205 entries (count-first TOC); part-stat / mission tables are in
lower-numbered entries; entry 201 (`0xC9`) holds the UI/menu **text** corpus
(~16k chars, `>`-terminated ASCII — see [DISC_TEXT_INVENTORY.md](DISC_TEXT_INVENTORY.md)).

### Mission text (`MIS.T`) — CRACKED
`MIS.T` is a count-first `.T` container (884 hdr, 883 entries) holding the entire
mission corpus: **entry 0** = mission-name array (0x20-byte stride, index = mission
number); per-mission **briefing/objective** text blocks (50, each begins with a
`Requester:` entry); plus 194 100×100 8bpp thumbnail TIMs. All text is ASCII and
`draw_string`-renderable. Full layout in [MIS_FORMAT.md](MIS_FORMAT.md); disc-wide text map
in [DISC_TEXT_INVENTORY.md](DISC_TEXT_INVENTORY.md). Tools: `tools/{build_filemap,extract_t,scan_text}.py`.

---

## 4. FDAT entry 201 — the text/menu overlay

Entry 201 (`0xC9`) is a code+data overlay loaded to **`0x8004ADA0`**.
- Disc: sectors **12475–12740** within FDAT (265 sectors, 542,720 B).
- Flat offset in extracted FDAT.T: **25,548,800** (`12475 × 2048`).
- Trailing checksum word at flat **26,091,516** (see §6).

### Rendering functions (in the overlay)
| Function | RAM addr | Entry-201 offset | Notes |
| --- | --- | --- | --- |
| `draw_char` | `0x800660C4` | `+0x1B324` | one glyph → `SPRT_VAR` (GP0 0x64) |
| `draw_string` | `0x8006599C` | `+0x1ABFC` | Shift-JIS walker; `>` (0x3E) terminator |
| `draw_kanji` | `0x80065DBC` | `+0x1B01C` | 2-byte SJIS path (dead in USA build) |
| (only `draw_string` caller) | `0x8009C1FC` | — | the menu command queue processor |

**Font texture** (MENU_TIM.T, embedded in entry 0 at file off `0x2E20`):
256×192 4bpp, VRAM (448,0) → **tpage 0x0007**; CLUT (368,224) → **CBA 0x3817**
(menu white), CLUT (352,240) → **CBA 0x3C16** (HUD colour).

**Glyph mapping:** `col = (code-0x20)&0x1F`, `U = col*8`, `row = (code-1)>>5`.

| size_mode | glyph | V formula | x-adv |
| --- | --- | --- | --- |
| 0 LARGE | 8×16 | `row*16` | 8 |
| 8 | 8×8 | `row*8+48` | 8 |
| 6 | 6×8 | `row*8+72` | 6 |
| 4 | 4×8 | `row*8+96` (+4 y) | 4 |

**Font texture V-band map (USA build — NONE of it is dead):**
`V0-47` LARGE, `V48-71` SMALL8, `V72-95` SMALL6, `V96-119` **SMALL4 (real)**,
`V120-191` **pre-baked English word-art** (MISSION/MAIL/GARAGE/RANKING/SHOP/SYSTEM).

**`draw_string` escape codes** (inline in the string):
| byte | effect |
| --- | --- |
| `>` 0x3E | terminator |
| 0x0D | CR: x=x_start, y+=16, skip next byte |
| `;` | newline: x=x_start, y+=16 (mode0) / +8 (modes 4/6/8) |
| space 0x20 | x+=8, no glyph |
| `{`+d / `}`+d | x −= d / x += d |
| `^`+d | size_mode = d |
| `~` | y −= 4 |
| `@`+d | palette = d |

**FontCtx fields:** `+0x08` zone (0–20 → style 0–9), `+0x0C` `str`, `+0x14`
z_depth, `+0x16` render_mode (`0x0015` → fixed colour, else GTE NCCS), `+0x18`
`ot[]` OT bases (per font_variant), `+0x48` x, `+0x4C` y.

**Key globals:** `prim_ptr` `0x801EF6CC`, `font_variant` `0x801EF6C8`, font
metrics base `0x801BCE88` (stride `style*100 + variant*103512`).

---

## 5. Name-entry engine (pilot / AC name)

Command dispatch: queue processor `0x8009C1FC` switches on cmd type at
`struct+0x0A` → 8 renderers (type7 = `draw_string` `0x8006599C`; type6 = GTE
renderer `0x80059B50`).

| Element | Address | Notes |
| --- | --- | --- |
| PILOT NAME entry ref | `0x800811F8` | entry-201 file off `0x36458` |
| AC NAME entry | `0x800865FC` | entry-201 file off `0x3B85C` |
| append routine | `0x80081950` | `rowstr + col*2`, **2-byte stride** |
| 5-row grid loop | `0x800814A0` | grid table `0x800B85F4` |
| 7-loop | `0x800810B0` | |
| state jump table | `0x8004C89C` | 5 states |
| label drawer | `0x8005D80C` | type7 |
| grid blit helpers | `0x8005CB94`, `0x8005D15C` | MoveImage VRAM blit |

**Cursor state** (per-screen struct `a1`): `+25` mode (`0x41`='A' → AC name,
else pilot), `+26` length (0–8), `+30` column, `+31` row.

**Name buffers (MAIN exe RAM):** AC name `0x80031BD4`, PILOT name `0x80031BE6`,
`>`-terminated, written at `len*2` (2-byte SJIS). Display uses `draw_string`, so
whatever bytes are stored render directly.

**Active grid rows** are 2-byte full-width Latin (row1 `0x8004C854`, row2
`0x8004C840`); the `0x8004C700` kana strings are leftover JP, not the live grid.

---

## 6. Overlay checksum — **CRITICAL GOTCHA**

**Every** container entry loaded via `read_T_entry` (`FUN_800165E4`) carries a
**trailing 32-bit checksum word**. The loader spins forever ("NOW LOADING" hang)
on mismatch. Confirmed on FDAT entry 201 **and** MENU_TIM.T entry 0; assume it
applies to all `.T` containers.

```c
// FUN_80015B24 @ main EXE 0x80015B24
sum = 0x12345678;                      // seed
for (i = 0; i < nwords - 1; i++)       // nwords = nsectors * 0x200
    sum += word[i];                    // 32-bit wrapping add
ok = (sum == word[nwords - 1]);        // trailing word = stored checksum
// read_T_entry loops while !ok  → infinite hang on mismatch
```

**Recipe (any entry spanning sectors `toc[i]‥toc[i+1]-1`):**
`nwords = (toc[i+1]-toc[i]) * 512;`
`checksum = 0x12345678 + Σ word[0 … nwords-2];`
write it to `toc[i]*2048 + nwords*4 - 4`.

- **FDAT entry 201:** trailing word @ flat `26,091,516`. Original `0x52BC6907`.
  Automated in `patch_draw_string.py → fix_overlay_checksum()`.
- **MENU_TIM.T entry 0:** base file off `0x800`, trailing word @ `0x117FC`.
  Automated in `build_pdigits.py → fix_entry0_checksum()`.

**Hang signature:** breakpoints never fire, registers frozen, only the stack
(~`0x801FFEB0`+) churns (CPU in the loader retry-spin, VBlank IRQ touching stack).
If an overlay edit hangs at load, suspect the checksum **first**.

---

## 7. Build & inject pipeline (CONFIRMED working)

```sh
# Compile (note: -march=r3000 REQUIRES -mfp32)
mipsel-linux-gnu-gcc -march=r3000 -mips1 -mfp32 -EL -G0 -O2 \
  -ffreestanding -fno-builtin -fno-pic -mno-abicalls \
  -Wl,--section-start=.text=0x8006599C -Wl,-e,<fn> \
  -Wl,--defsym=draw_char=0x800660C4 -nostdlib -o out.elf src.c
mipsel-linux-gnu-objcopy -O binary --only-section=.text out.elf out.bin
```
- `objcopy` needs `--only-section=.text` (else "huge negative file offset").
- Linker emits a leading 4-byte `nop` at `_ftext`; strip it when patching
  (extract from the real symbol offset) — **except** when the code has a
  position-dependent in-image table (e.g. packed `fmet[]`), then keep it.

**Patch + inject:**
1. Write the function bytes at its flat offset in `fdat_extracted.T`, NOP-pad
   (`0x00000000`) to the region end.
2. `fix_overlay_checksum()` (§6).
3. `psxinject "<bin>" GG/COM/FDAT.T fdat_extracted.T` — replacement file must
   equal the original size (keep entry 201 at 265 sectors). Use forward slashes,
   no leading slash. EDC/ECC handled by psxinject.

---

## 8. Farsi localization status

- **Renderer works** — `draw_string` byte `>=0x80` → `draw_farsi` → `SPRT_VAR`
  from a packed `fmet[]` table. Start menu translated (بازی جدید / ادامه), bold
  Noto Sans Arabic, RTL right-aligned (new escape `\x01 <signed byte>`).
- **128-glyph inventory** (`farsi_glyphs.py`): 32 Persian letters (D=4 forms,
  R=2 forms) + آ + lam-alef(2) + 10 digits, bytes `0x80–0xFF`. Space stays `0x20`.
- **Shaping:** build-time `farsi_shape.py` (+ validated runtime port
  `farsi_runtime_shape.py` / `farsi_name_shape.c`). Emits glyphs in RTL draw
  order, so the L→R `draw_string` renders correct RTL unchanged.
- **Open issue (VRAM conflict):** overwriting font rows `V96-191` broke the
  name-entry grid (2-byte SJIS) and garage word-art — that region is **not**
  dead. Fix = relocate Farsi glyphs to separate VRAM/tpage and restore originals.
- **Full-Farsi name keyboard** (planned): store *shaped* draw-order bytes in the
  existing name buffer so every display site works unchanged; change only the
  name-edit append routine (`col*2 → col*1`) + grid rows + cursor bounds.

See [AC1_TEXT_SYSTEM.md](AC1_TEXT_SYSTEM.md) for the full text/menu system writeup.

---

## 9. Disc layout (key files & sectors)

`SLUS-01323` v1.1, 2352-B/sector image. Sector ranges:

| File | Sectors | Size | Role |
| --- | --- | --- | --- |
| `SYSTEM.CNF` | 38 | 69 B | boot config (`BOOT = cdrom:\SLUS_013.23`) |
| `SLUS_013.23` | 39–121 | 166 KB | **main executable** |
| `GG/COM/FDAT.T` | 72189–85330 | 26.9 MB | game logic, part stats, menus, **text overlay (entry 201)** |
| `GG/COM/RTIM.T` | 85331–93643 | 17 MB | runtime TIM textures |
| `GG/MS/ARMS_T.T` | 93645–95437 | 3.7 MB | arms textures |
| `GG/MS/CORE_T.T` | 95732–96164 | 887 KB | core textures |
| `GG/MS/HEAD_T.T` | 96229–96709 | 985 KB | head textures |
| `GG/MS/LEG_T.T` | 96710–100294 | 7.3 MB | leg textures (largest MS asset) |
| `GG/MS/MENU_TIM.T` | 100295–101035 | 1.5 MB | menu UI textures (**font in entry 0**) |
| `GG/MS/MENU_TMD.T` | 101036–101818 | 1.6 MB | menu 3D models (TMD) |
| `GG/MS/MENU_VAB.T` | 101819–101919 | 207 KB | menu sound bank (VAB) |
| `GG/MS/MIS.T` | 101920–103449 | 3.1 MB | **mission text corpus** + 194 thumbnail TIMs (see `docs/MIS_FORMAT.md`) |
| `GG/P0–P3/PA00–PA71.T` | 103637–126704 | ~700–900 KB ea | **72** stage/map packs — geometry/textures, **no text** (`docs/PA_FORMAT.md`) |
| `GG/STR/ACED1-5.STR` | 131042–157931 | 4–14 MB ea | 5 ending FMVs |
| `GG/STR/ACOPA/B.STR` | 157932–176401 | 20+18 MB | opening cutscene (2 parts) |
| `GG/STR/DEMOPLAY.STR` | 176402–192619 | 33 MB | attract/demo video |
| `GG/STR/STAFF.STR` | 192620–209137 | 34 MB | staff roll |
| `GG/BGM/BGM00-01.XA` | 124+ | ~43+41 MB | stereo BGM (8-ch interleaved) |
| `GG/BGM/CPU00-23,10-13,20-23.XA` | — | ~3 MB ea | CPU arena music (mono, 16-ch) |

---

## 10. Quick address index

| Symbol | Address | |
| --- | --- | --- |
| entry point `start` | `0x80011E6C` | EXE |
| `main` | `0x80011F28` | EXE |
| game loop | `0x800122A8` | EXE |
| `load_T_file` | `0x80016678` | EXE |
| `read_T_entry` | `0x800165E4` | EXE |
| `read_sectors` | `0x80016328` | EXE |
| `wmemset` | `0x80015538` | EXE |
| checksum verify | `0x80015B24` | EXE |
| file-record table | `0x8004A2A8` | EXE RAM |
| game-mode flag | `0x80039C5E` | EXE RAM |
| state-handler fn ptr | `0x8004ADA4` | EXE RAM |
| entry-201 overlay base | `0x8004ADA0` | RAM |
| `draw_char` | `0x800660C4` | overlay |
| `draw_string` | `0x8006599C` | overlay |
| `draw_kanji` | `0x80065DBC` | overlay |
| menu queue processor | `0x8009C1FC` | overlay |
| name append routine | `0x80081950` | overlay |
| AC name buffer | `0x80031BD4` | EXE RAM |
| PILOT name buffer | `0x80031BE6` | EXE RAM |
| `prim_ptr` | `0x801EF6CC` | RAM |
| `font_variant` | `0x801EF6C8` | RAM |
| font metrics base | `0x801BCE88` | RAM |

---

## 11. Mission overlay & PA##.T loader (RE 2026-06-08)

The in-mission code is **FDAT entry 202 (`0xCA`)**, a 286,720-byte overlay loaded
to the same base as entry 201 (**`0x8004ADA0`**); it is therefore *not* in the
entry-201 Ghidra DB and must be imported separately for deeper work. Extract it
with `tools/extract/extract_t.py` (FDAT entry 202) → load at vma `0x8004ADA0`.

**PA stage loader** — `FUN_8004F1A8` (overlay):
- Reads stage/area number from byte `DAT_8004121B`, formats the path template
  `"P0\PA00.T"` (stored at `0x8008D928`) in place → `"Pn\PAnn.T"`.
- `load_T_file(0, path)` — **stage PA file uses file slot 0** (corrects §3).
- `read_T_entry(0, 1, dest)` — loads PA **entry 1** (the offset directory) first;
  later sites read the geometry blocks (entries 2..N).
- Calls the **resource registrar** `FUN_80073AD8(record, dest, index)`.

**Resource registrar** — `FUN_80073AD8` (overlay):
- `memcpy`-allocs each PA block into mxt work RAM (`jal 0x8002A480`, allocator ptr
  `DAT_801A5DB8`), word-aligns the allocator.
- Writes a **44-byte record** into the table at `0x8019F538` (stride 44, indexed by
  block #): `+0x28` = work-RAM pointer, `+0x00`/`+0x02` = element counts
  (`= halfword[block+4]>>2` and `halfword[block+6]>>2`), `+0x04` = block index.
- Dest-ptr holder `0x8019F520`; per-PA state base `0x8019F518`.

**Geometry walker / renderer (RE 2026-06-08 — see `docs/PA_FORMAT.md`):**
- **`FUN_800574D8`** — relocation walker. Called with `a0 = block + block[+8]`; the
  **stride-28 sub-object table** is at `a0+12` (= `block + block[+8] + 12`), entry
  count `a0[+8]`. Per descriptor it relocates the in-table offsets (`field += a0+12`)
  and dispatches each primitive record through jump table **`0x8004B184`** (157
  entries, index = `((firstword>>24 & 0xBC) − 0x20)`, 16 distinct handlers @
  `0x800575C0`…`0x80057944`); converts raw vertex indices → byte offsets (`<<3` ×8
  into stride-8 XYZ vertex pool, `<<4` ×16 into colour/normal pool). One-time, in
  place. **Sub-object descriptor (28 B):** `+0`=vtx-pool off, `+4`=vtx count,
  `+8`=colour/normal pool off, `+0x0e`=flags (`0x8000`=skip, low9=count addend),
  `+0x10`=prim-stream off, `+0x14`=prim count base, `+0x18`=4th pool off.
  Prim records walked = `u16[+0x14] + (flags&0x1ff) − 1`. (Decoder/OBJ exporter:
  `tools/pa/pa_obj.py`. NB: relocation adds +0x18 file-relative — the true PA00 e2
  vertex pool base is **0x1230**, not the previously-noted 0x1218.)
- **`FUN_80057C44`** — per-sub-object (124-byte stride) bbox transform + NCLIP cull.
- **`FUN_8005A57C`** — per-frame **primitive emitter** (the GPU walker). Dispatches
  on `(type & 0xFD)` → tri/quad/gouraud/textured branches, fetches transformed
  screen XY from pool (`s6 + idx`), backface-tests, builds POLY packets and OT-links
  them. Matrix-driven variant `FUN_80058B04` (called from `0x5D618`/`0x5E1C8`).
- **`FUN_80078B14`** — per-object setup: copies an object-template record into a
  ~0x168-byte display struct. The `÷44` magic-multiply (`0xE9BD37A7`) on
  `(a0 − 0x801A26B8)` computes the **display-slot index (1..16)** — *not* a block index
  (corrects the earlier note). The **block index is template field `a1[+0x0A]`**;
  it indexes the 44-byte record table (`blkrec = 0x8019F538 + a1[+0x0A]*44`) and the
  work-RAM geometry ptr `blkrec[+0x28]` is copied to `display[+0x80]`.

**Entry-0 / entry-1 + slot binding (RE 2026-06-08 — see `docs/PA_HEADER.md`):** the
PA loader reads **entry 1 FIRST** (the placement directory) — `read_T_entry(0,1,dest)`
@`0x8004F244` — and the registrar stores it as block record **[0]** (the table at
`0x8019F538` therefore holds the directory, not the 112 geometry blocks). **Entry 0**
= `used / sig 0x03072D39 / 4-ptr table (+0x08) / typed object-index list (+0x1C)`; the
list is `count(0x71)` then `(blockID<<8)|subtype` entries (45 `0xNN08` block-roster
entries + paired `0xNN03`/`0xNN02` group entries) and is **byte-identical across
PA00/PA07/PA20** → fixed slot roster. **Entry 1** = `used / count 57 / uint16 ptr
table (+0x08)` to 57 placement records (marker `0x012C/0x0352` + int16 transform
vectors), table also byte-identical across files. The object-instance table at
`0x8019FAB8` (256×44, init `FUN_80073B74`) is filled from the **mission** file (id 2),
and each instance's field `+0x0A` selects its PA geometry block → **slots are
data-driven but authored from a fixed template**. Header dumper: `tools/pa/pa_parse.py
… --header`.

**Primitive record (CONFIRMED):** variable length, `reclen = 4 + byte[1]*4`;
`byte[3]&0xBC` = type (0x80 bit = textured). Layout = word0 / shading block (flat =
1 RGB+code word `c8 c8 c8 cc`; textured = UV+clut, UV+tpage, UV…) / N uint16 vertex
indices / optional flag word. Index offsets per type: 0x20→+8(3v), 0x28→+8(4v),
0x24→+0x12(3v), 0x2c→+0x14(4v), 0x34→+0x12(3v), 0x3c→+0x12(4v). Validated on
PA00 e2 @0x1960 and PA20 e3 @0x6c (indices in-range, walk hits next section).
Decoder: `tools/pa/pa_parse.py … --prims OFF CNT`.

| Symbol | Address | |
| --- | --- | --- |
| mission overlay (entry 202) base | `0x8004ADA0` | overlay |
| PA stage loader | `0x8004F1A8` | overlay |
| PA resource registrar | `0x80073AD8` | overlay |
| PA per-object setup (reads record table) | `0x80078B14` | overlay |
| PA relocation walker | `0x800574D8` | overlay |
| PA primitive jump table (157) | `0x8004B184` | overlay |
| PA sub-object cull/transform | `0x80057C44` | overlay |
| **PA primitive emitter (GPU walker)** | `0x8005A57C` | overlay |
| PA primitive emitter (matrix variant) | `0x80058B04` | overlay |
| stage/area number byte | `0x8004121B` | RAM |
| PA path template `"P0\PA00.T"` | `0x8008D928` | overlay |
| PA block record table (stride 44; [0]=entry-1 dir) | `0x8019F538` | RAM |
| PA dest-ptr holder | `0x8019F520` | RAM |
| PA loader top-level caller (a0=0) | `0x8004FA00` | overlay |
| object-instance template table init | `0x80073B74` | overlay |
| object-instance template table (256×44) | `0x8019FAB8` | RAM |
| display-struct pool (16×0x170) | `0x801A2828` | RAM |
| display-slot index base (for ÷44 magic) | `0x801A26B8` | RAM |
| free display-slot allocator | `0x80078A2C` | overlay |

---

## 12. Mission runtime — descriptor, MT spawn, timer, objectives (RE 2026-06-08)

Full writeup: **`docs/MISSION_SYSTEM.md`**. Tool: `tools/mission/mission_parse.py`. All in
the entry-202 overlay (base `0x8004ADA0`).

**Per-mission data = FDAT entry PAIR (file id 2):** mission N →
- **entry `2N`** = the **objective object** (relocatable MIPS code+data loaded to
  `0x801C4B40`; starts with a **method vtable**; objective logic is per-mission
  CODE, not a global enum).
- **entry `2N+1`** = the **chunk stream** (`[u32 len][payload]…`), walked by the
  scene loader **`FUN_8004F508`**. **Chunk 12 = 256×40-byte MT/object spawn table**
  → `FUN_80073B74` → instance table `0x8019FAB8`.

**Spawn record (40 B = 20×int16):** `hw0-2`=X,Y,Z; **`hw3`=geometry block index**
(→ instance `+0x0A`, `-1`=none); **`hw7`=object/MT type id**; `hw5`=rot (hyp);
`hw8..19`=per-type params (hyp). Binding via `FUN_80078B14` (PA_HEADER §setup).

**Timer:** displayed `0x8019F52C` (frames, MM:SS HUD via ÷3600 magic); set by
script **cmd 4** / `FUN_8008A778`. End-anim counter `0x8019F52A`. Per-frame driver
**`FUN_8008AB68`** → timer state machine **`FUN_8008A8F8`** (state 60 special).
Timer-tick handler (`~0x8004C4F0`) forces FAIL on expiry. Timer struct `0x801D0B40`.

**Objective / success-fail:** result FLAGS word **`0x8019F524`** — bit `0x100`=
**SUCCESS**, `0x200`=**FAIL** — read at mission exit (`0x8004C69C`) → result code
**`0x80048610`** (1=success,2=fail). End primitive **`FUN_8004C318(a0)`** writes the
flag + arms the 100-frame exit fade (`0x8019F528`). Objective-step primitive
**`FUN_8008A80C`** drives progress counter `DAT_8009079C` (terminal at 36). PRIMARY
script VM **`FUN_8008A0B0`** + 10-entry jump table **`0x8004C164`** (cmd2=load
secondary-VM set, cmd4=set-timer, cmd5=flag 0x80, cmd8=prompt-gated success,
timer-tick=0x200).

The **secondary actor-thread VM** (chunk 4 of the stream; tick `0x8008B380`,
dispatch `0x8008B42C`) is the data-driven scripting language — 22 opcodes incl.
`set_result` (`0x100A`→`FUN_8004C318`), scripted spawns, and movement/lerp. Full
decode + assembler spec: **`docs/MISSION_SCRIPT_VM.md`**; type→behaviour binding +
objective conditions: **`docs/ENTITY_TYPES.md`**; the level/mission **write-path**
(edit→repack→reinject): **`docs/AUTHORING.md`**.

> **Corrections (2026-06-15):** `FUN_80052A2C` is the TEXT/PROMPT VM (cmd 8 is a
> prompt-gated success), not a "condition 99" predicate. `0x8008BAF8` is the
> secondary-VM `set_result` opcode body, not a separate VM. Ready gate = `FUN_80052338`.
> `hw7` (model id, 0..388) and the logical dispatch type (entity `+0x0E`, 1..6) are
> **different namespaces** (`ENTITY_TYPES.md`).

| Symbol | Address | |
| --- | --- | --- |
| mission scene loader | `0x8004F508` | overlay |
| mission control block ptr→objective object | `0x8019F51C` | RAM |
| objective object load addr | `0x801C4B40` | RAM |
| result/objective FLAGS word | `0x8019F524` | RAM |
| mission-END seq counter (→exit) | `0x8019F528` | RAM |
| timer-mode / end-anim counter | `0x8019F52A` | RAM |
| **displayed mission TIMER (frames)** | `0x8019F52C` | RAM |
| mission result code (1=ok,2=fail) | `0x80048610` | EXE RAM |
| objective step counter | `0x8009079C` | RAM |
| mission number / phase byte | `0x800411F8` | RAM |
| timer-display struct | `0x801D0B40` | RAM |
| **mission-end primitive (set flag+fade)** | `0x8004C318` | overlay |
| objective-step primitive (advance) | `0x8008A80C` | overlay |
| per-frame mission driver | `0x8008AB68` | overlay |
| timer/end state machine | `0x8008A8F8` | overlay |
| mission script interpreter | `0x8008A0B0` | overlay |
| mission script jump table (10 cmds) | `0x8004C164` | overlay |
| timer set helper (cmd 4) | `0x8008A778` | overlay |
| "MISSION TIMER" / "#Location Now" strings | `0x8004ADB0` / `0x8004C1AC` | overlay |

---

## 13. Player interactions & live mission ground-truth (RE 2026-06-14)

Live (DuckStation) RE of the in-mission interactive systems. Writeups:
**`docs/INTERACTIONS.md`** (Circle-context door/item, COM dialogs, SsVm sound) and
**`docs/MISSION_SYSTEM.md` §6** (completion flow, spawn-activation chain). Overlay
provenance + the annotated Ghidra project: **`overlays/OVERLAY_MAP.md`** (isolated
`ghidra_mission202/Mission202.gpr`).

- **Level completion is two-step:** destroy last objective → `0x801D0B50` (=struct
  `0x801D0B40`+0x10) `= 3` *objectives-complete* + a "last objective" COM dialog
  (does **not** end the mission); then **cross the exit border** → script cmd 5 →
  `FUN_8004C318(0x80)` → arms `0x8019F528=100` → debrief. End-primitive
  `FUN_8004C318` and the cmd-5 path are now **live-confirmed**.
- **Sliding door** = world-structure object (`0x801D0B68` array, stride `0x40`);
  state word `+0x30` **bit `0x100`=closed**, cleared by Circle-interact; open
  routine `0x801C6B68` (objective overlay).
- **Item pickup** ("COM : AC weapon obtained") = SsVm sound + COM dialog; obtained
  part deferred to the owned-parts save at mission end (no live mid-mission flag).
- **SsVm sound engine** (resident): trigger `UT_KEYV_OBJ_180` `0x80021880`, voice
  table `0x80040938` (stride `0x1A`) — a memory-diff **red herring** (fires on any
  Circle press).
- **Method:** idle-baseline diff subtraction + aligned-word watchpoints; for a
  per-frame-rewritten flag, NOP the writer's store, catch the real setter, restore
  (see the `mem-diff-baseline` skill). The per-frame `0x8019F524` masker is the
  `sw` at `0x8008A938` in `FUN_8008A8F8`.

| Symbol | Address | |
| --- | --- | --- |
| objectives-complete state (struct `0x801D0B40`+0x10) | `0x801D0B50` | RAM |
| door/world-structure object array | `0x801D0B68` | RAM |
| door open routine (objective overlay) | `0x801C6B68` | overlay |
| per-frame `0x8019F524` masker (`sw`) | `0x8008A938` | overlay |
| SsVm sound trigger | `0x80021880` | EXE |
