# PA##.T entry-0 (master header) + entry-1 (object/placement directory)

Target **SLUS-01323 (v1.1)**. Code-side companion to [PA_FORMAT.md](PA_FORMAT.md) (container +
geometry blocks), [PA_SLOTS.md](PA_SLOTS.md) (empirical cross-file slot matrix), and
[REFERENCE.md](REFERENCE.md) §11 (loader addresses). Offsets verified on the **pristine** bin
against **PA00 / PA07 / PA20** (`GG/P0` + `GG/P1`). Overlay code = FDAT entry 202,
base `0x8004ADA0`.

> Terminology: `docs/PA_SLOTS.md` calls a `.T` *entry index* a "slot". The entry-0
> list below uses an internal **blockID** tag (0x00..0x2c). These are two views of the
> same fixed roster — the header is *why* PA_SLOTS sees a fixed skeleton.

## TL;DR — why every PA file shares the same block-layout skeleton
The invariance PA_SLOTS.md measures empirically is **explained by a fixed header
skeleton baked into entry 0 and entry 1**:

- **Entry 0** holds a `count`-led **typed object-index list** of `(blockID<<8)|subtype`
  entries. This list is **byte-for-byte identical across PA00 ≡ PA07 ≡ PA20**: same
  block IDs, same order, same subtypes. → the block-ID → roster-position mapping is
  **fixed across PA files**.
- **Entry 1** is a 57-record **placement/object directory**; its uint16 pointer table
  at `+0x08` is byte-identical for the first ~40 records across all three files, and
  every record opens with the same `0x012C / 0x0352` marker. → the directory *shape*
  is fixed; only the int16 transform/coordinate payloads differ per file.

So PA slots are **data-driven by a fixed authored template**: the *set of slots and
their order* is effectively hardcoded (identical bytes in every file), while each file
supplies different per-slot geometry + placement vectors. This matches PA_SLOTS.md's
"FIXED role filled with per-stage data".

---

## Entry 0 — master header (CONFIRMED layout)

`u32[0]` = used byte size (PA00 `0xF854`; file padded to 0x10000). `u32[1]` =
signature `0x03072D39`. Then a 4-entry pointer table, then the typed list.

| off | type | field | status |
| --- | --- | --- | --- |
| `+0x00` | u32 | used size (≤ entry length) | CONFIRMED |
| `+0x04` | u32 | signature `0x03072D39` | CONFIRMED |
| `+0x08` | u32 | ptr A → tail region (`0xF0BC` in PA00): sparse `0x8000`-flag array | CONFIRMED ptr; payload HYPOTHESIS |
| `+0x0C` | u32 | ptr B = `0x1C` → start of the typed object-index list (below) | CONFIRMED |
| `+0x10` | u32 | ptr C = `0x188` → **u32 offset directory** (ascending `0xE4,0x284,0x6FA,…`) into the entry-0 record region | CONFIRMED ptr; per-object record offsets |
| `+0x14` | u32 | ptr D (`0xEDC4` PA00) → region of signed int16 vectors/matrices (`f4e4 fd56 fce5 …`) = per-object transforms | CONFIRMED ptr; payload HYPOTHESIS |
| `+0x18` | u32 | aux ptr (`0xEFD4` PA00), just past ptr-D region | CONFIRMED ptr |

### `+0x1C` typed object-index list (CONFIRMED structure; byte-identical across files)
```
u16 count = 0x71 (113)                                 CONFIRMED
then `count` typed u16 entries = (blockID << 8) | subtype:
  phase A (45 entries): subtype 0x08  — blockID 0x20,0x24,0x1d,…,0x01,0x00
  phase B (paired):     subtype 0x03 + u16 child-count, interleaved with
                        subtype 0x02 markers (e.g. 1602:0  1603:0x2a  1502:1 1503:0x2c …)
```
- The **45 `0xNN08` entries** = one per distinct geometry blockID (max blockID 0x2C ⇒
  45 = 0x2D slots). This is the **blockID → slot roster**.
- The trailing `0xNN03`(count) / `0xNN02` pairs associate each blockID with a child /
  instance count — an **object-tree / LOD / sub-part grouping** (HYPOTHESIS).
- **Cross-file invariant:** PA00 ≡ PA20 ≡ PA07 for the entire list (verified byte-equal
  via `tools/pa/pa_parse.py … --header`). Core evidence that slot roles are fixed.

## Entry 1 — object/placement directory (CONFIRMED container; payload partial)

`u32[0]` = used size (PA00 `0x5E4`). `u32[1]` = **record count `0x39` = 57**.

| off | type | field | status |
| --- | --- | --- | --- |
| `+0x00` | u32 | used size | CONFIRMED |
| `+0x04` | u32 | record count = 57 | CONFIRMED |
| `+0x08` | u16[57] | **pointer table** — file-relative offsets to the 57 records | CONFIRMED |
| record | var | per-object placement record (below) | partial |

- The uint16 pointer table is **byte-identical** PA00/PA07/PA20 for the first ~40
  entries (`0x190, 0x352/0x384, 0x5DE/0x5E4, 0x0000, 0x0072, 0x0082, 0x00A2, …`).
- **Record content (HYPOTHESIS, byte-read):** records begin with a repeated marker
  `2c 01 52 03` (= `0x012C`, `0x0352`), then `(blockID, count, …)` halfwords, then runs
  of **signed int16 vectors** (`53 ff 06 00 00 00 … 8b fe`) = per-instance translation /
  placement vectors. The first three pointer entries (`0x190`, `0x352`, `0x5DE`) address
  the large vector blocks whose size grows the file.

Entry 1 is the **first** thing the loader reads (before geometry blocks) and the
registrar copies it verbatim into work RAM → it is the master placement table the
renderer walks per object.

---

## Code path — how a block binds to an object slot (CONFIRMED addresses)

FDAT entry-202 overlay, base `0x8004ADA0` (extract `tools/extract/extract_t.py`; disassemble
`mipsel-linux-gnu-objdump -b binary -m mips:3000 -EL --adjust-vma=0x8004ADA0`).

### 1. PA loader `FUN_8004F1A8` (0x8004F1A8) — called once, `a0 = 0` (from 0x8004FA00)
- Stage byte `DAT_8004121B`, ÷10 magic (`×0xCCCCCCCD`) → format `PAnn` → path
  `"Pn\PAnn.T"` → `load_T_file(0, path)` (`jal 0x80016678`).
- `read_T_entry(0, 1, dest)` (`jal 0x800165E4`), `dest = mem[0x8019F520]` — loads
  **entry 1 (the placement directory) FIRST**.
- `FUN_80073AD8(record = 0x8019F538 + 0*44, dest, 0)` — registers entry 1 as block
  record **[0]**.
- Then up to 8 conditional sub-resource loads keyed on stage bytes
  `DAT_8004121D..0x1224` (`jal 0x800537FC`), stored at slot indices `s0*16 + 224..231`.

### 2. Registrar `FUN_80073AD8` (0x80073AD8)
`memcpy`-allocs the block into mxt work RAM (`jal 0x8002A480`, allocator ptr
`DAT_801A5DB8`) and writes a 44-byte record at `0x8019F538 + idx*44`:
`+0x28` = work-RAM ptr, `+0x00 = u16[block+4]>>2`, `+0x02 = u16[block+6]>>2`,
`+0x04 = idx`. **Only called once (idx 0)** → the 44-byte table at `0x8019F538` holds
the entry-1 directory pointer, not the 112 geometry blocks individually.

### 3. Object-instance template table `FUN_80073B74` (0x80073B74)
Init for the **256-record ×44 object-instance table at `0x8019FAB8`**, filled from the
**MISSION** file (id 2) chunk stream (not PA): per record, `byte[0]=0`, 20 halfwords
copied from source, `rec[+0] = rec[+0x18]`. These records are the per-object instances
placed in the stage.

### 4. Per-object setup `FUN_80078B14` (0x80078B14) — THE binding
- `a0` = display struct (one of 16 at `0x801A2828`, stride `0x170`), `a1` = object
  template record.
- `slot = (a0 − 0x801A26B8) × 0xE9BD37A7 >> (32+4)` — **this magic-÷44 computes the
  display-slot index (1..16), NOT a block index.** (Corrects the earlier note in
  `PA_FORMAT.md` / REFERENCE §11 that called it a "block index".) Result → `a1[+1]`.
- **Geometry binding:** `blkrec = 0x8019F538 + (s16 a1[+0x0A]) × 44`; reads
  `blkrec[+0]`,`[+2]`,`[+4]`,`[+0x28]` and copies the work-RAM ptr into the display
  struct at `a0[+0x80]`. So **object-template field `+0x0A` is the block index** into
  the registrar table — the link from object instance → geometry block.
- Other template→display copies: `a1[+0x1A]→a0[+0x162/+0x164]`, `a1[+0x02]→a0[+0x160]`,
  `a1[+0x04..+0x0A]→a0[+0x08..+0x0E]`, two `lwl/lwr` 9-byte transform blocks
  (`a1[+0x22..]`, `a1[+0x0C..]`).

### Free-slot allocator `FUN_80078A2C` (0x80078A2C)
Scans 16 display structs at `0x801A2828` (stride 0x170) for a free one; parallel
16-entry handle table at `0x801A2828` and instance-record table at `0x801A26B8`.

## Are slot roles FIXED or DATA-DRIVEN?  →  **Data-driven by a fixed template (both)**
- The *binding mechanism* is **data-driven**: an object instance names its geometry by
  template field `+0x0A` (block index) and its placement comes from the entry-1
  directory / mission instance records — no hardcoded block→object switch.
- The *roster* is **effectively fixed**: the entry-0 typed list and the entry-1
  directory shape are **byte-identical across files**, so every PA file is authored
  from the same slot skeleton. That is exactly why PA_SLOTS.md sees the same slot
  holding the same kind of object in every file.

## CONFIRMED vs HYPOTHESIS
- **CONFIRMED:** entry-0 = used/sig/4-ptr/typed-list; typed list = `count` then
  `(blockID<<8)|subtype`, **byte-identical PA00/07/20**. entry-1 = used/count(57)/u16
  ptr table, table byte-identical for the first ~40 records. Loader/registrar/setup
  addresses; `a1[+0x0A]` = block index binding; magic-÷44 = display slot (1..16).
- **HYPOTHESIS:** meaning of the entry-0 `0xNN03`(count)/`0xNN02` phase (object-tree /
  LOD); entry-1 record body schema beyond the `0x012C/0x0352` marker + int16 vectors;
  payloads behind entry-0 ptrs A/C/D.
