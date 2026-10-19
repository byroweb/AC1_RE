# Mission overlays as code units (FDAT_T even entries)

Target **SLUS-01323** (code-identical to SCUS-94182). This note treats the per-mission
"objective object" overlays as *code*: where they load, how their header is really
called, which program overlay owns which group of them, and what a byte-matcher needs
to compile against them. It reconciles the header descriptions in
[MISSION_SYSTEM.md](MISSION_SYSTEM.md), [ENTITY_TYPES.md](ENTITY_TYPES.md),
[MISSION_STATE_MACHINE.md](MISSION_STATE_MACHINE.md) and [MISSION_STAGES.md](MISSION_STAGES.md)
and cites them rather than repeating them. Load-sequence facts come from
[LEVEL_LOADING.md](LEVEL_LOADING.md) and `disc_map/trace/mission_load_sequence.md`.

Everything below was read from the disc bytes on 2026-10-18 with
`mipsel-none-elf-objdump -D -b binary -m mips:3000 -EL --adjust-vma=<base>` on
`disc_map/extracted/FDAT_T/entries/NNN.bin` and `disc_map/overlays/ovl20N_*.bin`, plus
`tools/loop/inventory.py --missions` (section e). Ground-truth section layout is
`overlays/CODE_UNITS.tsv` on branch `claude/xenodochial-mclean-55b2b3` (Wyrelade's v1.0
splat layout, 63 units; not merged at the time of writing).

Notation: **CONFIRMED** = read from the bytes at the cited address; **INFERRED** = follows
from confirmed facts but was not executed/observed; **HYPOTHESIS** = plausible, untested.

---

## a. A mission overlay as a code unit

### File → load address → sections  (CONFIRMED)

`FDAT_T` entry `2N` is a raw memory image: no header, no relocation table, no symbol
table. The program overlay reads it with the `.T` entry reader
`FUN_800165E4(container=2, entry, dest)` straight to a fixed address, and every
pointer inside it is already the final RAM address.

| load group | program overlay | entries | load address | image sizes |
|---|---|---|---|---|
| mission (story) | `fdat202` "mission" | even 0..98 (no 32/58/78), 120, 140 | `0x801C4B40` | 0x1800..0xA800 |
| mode2 (local 2-AC arena) | `fdat204` "mode2" | 160, 162, 164, 166, 168 | `0x801C8460` | 0x1000..0x1800 |
| mode1 (link-cable versus) | `fdat203` "mode1" | 180, 182, 184, 186, 188 | `0x801B80C8` | 0x1000..0x1800 |

Sections, from `CODE_UNITS.tsv` (`rodata_off`/`text_off`/`data_off`), checked against the
bytes of entry 0 (size 0x8000, loaded at `0x801C4B40`):

```
file 0x0000  +0x000  method-pointer header, 0x120 bytes (section b)
     0x0120  +0x120  switch jump tables and other rodata of the mission's own code
     0x0780  +0x780  .text   0x801C52C0 .. 0x801CAB98   (47 functions)
     0x6058  +0x6058 .data   0x801CAB98 .. end of image (static tables; M0's INIT indexes
                             a 0x544-byte-stride table at 0x801CAB98 via `lui 0x801D /
                             addiu -21608`, see f)
```

For the 16x/18x groups the header is exactly 0x120 bytes and `.text` starts at
`+0x120` (every one of their `text_off` values is 0x120); they carry no jump tables.
Image sizes are multiples of 0x800 (sector padding); the last word of several images
is a non-zero trailer (`188.bin` ends `C0 65 F0 16`, `166.bin` ends `93 41 6B B2`) that
is neither code nor a pointer — treat it as the `.T` entry checksum word
([MIS_FORMAT.md](MIS_FORMAT.md) mentions the same trailing word on `.T` entries;
INFERRED).

### "Relocatable" vs absolute: settled, absolute  (CONFIRMED)

`MISSION_SYSTEM.md` §TL;DR calls the objective object "relocatable MIPS code+data";
`ENTITY_TYPES.md` says "always loaded to 0x801C4B40 (no relocation; file pointers are
absolute)". The bytes support the second:

- Entry 0, header word 0 = `0x801C5594` = `0x801C4B40 + 0xA54`. File offset 0xA54 holds
  `27BDFFE8 AFBF0010 0C022A03` = `addiu sp,sp,-24; sw ra,16(sp); jal 0x8008A80C` — a
  function prologue that calls the ovl202 objective-step primitive. A relocatable
  image would store `0xA54` here; it stores the RAM address.
- All 0x140 header words of all 58 entries are either 0 or an address inside the
  entry's own image at its group's base (scan of every entry; the only "outside"
  values are code words, because in the 16x/18x groups `.text` begins at +0x120).
- The mission code calls the program overlay by absolute `jal`: entry 0's 47 functions
  contain 77 distinct `jal` targets outside the image, of which 66 are function starts in
  ovl202 and 10 in the exe, none in ovl203; entry 180's 4 external targets are all in
  ovl203; entry 166's single one is in ovl204. So an entry is bound to one program
  overlay *and* one address — it cannot be loaded anywhere else.
- The loader itself (`0x8004F77C`: `lui a2,0x801c; addiu a2,a2,0x4b40; jal 0x800165E4`)
  performs no fix-ups after the read.

Only the *linker* placed each mission's copy of the shared code at a different
address (next paragraph); nothing is relocated at load time.

### The same source, linked 58 times  (CONFIRMED by masked comparison)

`inventory.py --missions` splits the 58 images into 2769 functions. Comparing their
words with `lui`/`addiu`/load/store immediates and `jal` targets masked gives only
**362 distinct bodies**; **2626 of the 2769 functions have a twin in another entry**. The
largest function (4912 bytes, 12 jump tables, `func_801C981C` in entry 0) occurs in 29
entries; a block of ~30 functions (2004, 1376, 1296, 908, 892, 880, 844, 772 … bytes)
occurs in 31 entries at per-entry addresses. Only 72 functions are byte-identical *and*
at the same address in another entry. Reading: FromSoftware linked a common
"objective object library" into every mission together with the mission-specific
methods; the library lands wherever the mission's own code leaves it. For matching
this means one C file can serve up to ~30 units, differing only in `--defsym` values.
Masking only the immediates that carry a relocation (`tools/loop/twins.py group`) gives
**393** groups: the coarser mask above also merges functions that differ in struct
offsets, frame sizes or `li` constants and so need different C. `twins.py replay`
recompiles each matched mission C file against its clones (tools/loop/README.md).

---

## b. The method-pointer header, reconciled  (CONFIRMED unless marked)

The header is 0x120 bytes: eight fixed slots, then three pointer tables. Every entry
fills unused fixed slots with the same empty function (entry 0: `0x801C558C` =
`jr ra; nop`, entry 188: all eight = `0x801B81E8`). The table below says what the
program overlays actually call; "lw site" is the instruction that loads the slot, the
containing function is from `tools/loop/functions.tsv`.

| slot | ovl202 lw site (in function) | arguments | ovl203 / ovl204 lw site | name used here | in the docs |
|---|---|---|---|---|---|
| `+0x00` | `0x8008AB78` in `FUN_8008AB68` (per-frame driver, called from the frame loop at `0x8004C480`) | none | `0x8004C2EC` in `0x8004BF94` / `0x8004C188` in `0x8004BF3C` (the mode main functions) | **TICK** | all three agree |
| `+0x04` | `0x8004C404` in `0x8004C340` (ovl202 main): straight-line, after `FUN_8004F508` (scene loader), `0x800835A0`, `0x8008A344`, `0x800523DC`, `0x80082C58`, `0x8008D8DC`, and *before* the frame loop that starts at `0x8004C46C` | none | no load of `+0x04` in ovl203 or ovl204 | **START** (once per mission, after every loader has run) | `MISSION_SYSTEM.md` §5 lists `+4` as "called by the runtime" without naming it; `ENTITY_TYPES.md` omits it. Entry 0 points it at the empty stub; entries 2, 4, 6, 12, 14, 22, 24, … point it at a function distinct from `+0x14` |
| `+0x08` | `0x8008A290` in `FUN_8008A0B0` (primary-VM command 9) | `a0 = id & 0xFFFF` | `0x80088ED0` in `0x80088D2C` / `0x80088C80` in `0x80088ADC` | **EVENT(id)** | `ENTITY_TYPES.md` ✓ |
| `+0x0C` | `0x8008AF84` in `0x8008AF2C` | `a0 = &w0, a1 = &w1`, two zeroed stack words | no load in ovl203/ovl204 | **QUERY(out0,out1)** | `ENTITY_TYPES.md` "objective/HUD query". The caller skips the call when the mission byte `0x8019F518` is 0 or 70 and afterwards tests `0x8019F524 & 0x80`, so this is the end-of-mission result evaluation (INFERRED) |
| `+0x10` | `0x80078E7C` in `0x80078E1C`, followed by `FUN_80078D78(entity, rec)` (despawn) | `a0 = entity` | `0x80077BFC` in `0x80077B9C` / `0x80077A1C` in `0x800779BC` | **FREE(entity)** | `ENTITY_TYPES.md` ✓ |
| `+0x14` | `0x8004F9E8` in `FUN_8004F508`: right after the chunk walk, before `0x8019F52C` is zeroed and the PA bootstrap `0x8004F1A8` runs | none | `0x80050200` in `0x8004FE84` / `0x8004FCF8` in `0x8004F964` (same position in the mode loaders) | **INIT** | `MISSION_STATE_MACHINE.md` step 6, `MISSION_STAGES.md`, `MISSION_SYSTEM.md` §1 ✓; `ENTITY_TYPES.md` omits it. Entry 180 has a real `+0x04` (`0x801B86B4`) and an empty `+0x14` — defined, never called in mode1 |
| `+0x18`, `+0x1C` | none | — | none | reserved | not called anywhere; always the stub |
| `+0x20..+0x5C` | `0x80078D50` in `FUN_80078CFC` (spawn), index = `lh entity+0x0E` | `a0 = entity` | `0x80077AD0` in `0x80077A7C` / `0x800778F0` in `0x8007789C` | **INIT_BY_TYPE[16]** | `ENTITY_TYPES.md` ✓ (only ids 1..6 used) |
| `+0x60..+0x9C` | `0x80072090` in `FUN_80071E14`, index = `lbu rec+0x31`, skipped when 0xFF, gated on `lh rec-0x06 != -1` | `a0 = s2` (the bound object) | `0x80071414` in `0x80071198` / `0x80071250` in `0x80070FD4` | **METHOD_B[16]** | `ENTITY_TYPES.md` ✓ |
| `+0xA0..+0x11C` | `0x800732EC` in `FUN_800731E8` (registrar), index = `lh rec-0x16`, null-checked | `a0 = s2` | `0x80072670` in `0x8007256C` / `0x800724AC` in `0x800723A8` | **METHOD_C[32]** | `ENTITY_TYPES.md` ✓ |
| `+0x120..` | no program overlay loads it through the object pointer | — | — | **not a method table**: in every ovl202-group entry `+0x120` (`0x801C4C60`) is the first *switch jump table* of the mission's own code (`inventory.py` finds `jr` through `0x801C4C60` in entry 0's `func_801C59EC`, and the same in all 50 entries of the group); in the 16x/18x groups `+0x120` is where `.text` starts | `ENTITY_TYPES.md` "AUX_TBL[8] (HYP)" — retire |

Contradictions settled: `+0x04` and `+0x14` are **both** real and distinct
(START after all loaders vs INIT inside the scene loader); `MISSION_SYSTEM.md` §5's
"+0/+4/+8/+0x14" is the correct set of fixed slots ovl202 calls, plus `+0x0C` and
`+0x10` which it lists under other names. Still **HYPOTHESIS**: what the two QUERY
output words mean, and whether any mission's START does more than the empty stub in
practice (entry 2's `0x801C698C` is a candidate to read first).

Where the object pointer lives: ovl202 `0x8019F51C` (stored at `0x8004F534`), ovl204
`0x801AB67C` (stored at `0x8004F974`), ovl203 `0x8019B484` (stored at `0x8004FEA4`).
Nothing else writes them. The 32 KB directly below the ovl202 base
(`0x801BCB40..0x801C4B40`) is zeroed (8 KB) and handed to `0x8002A150(ptr, 0x8000)` at
`0x8004C374..0x8004C394` in the ovl202 main function before the scene loader runs —
a per-mission work arena adjoining the object (INFERRED).

---

## c. The three load groups by game mode  (CONFIRMED)

The three loaders are the same code pattern (chunk-walk included); only globals and
bases differ.

| | ovl202 mission | ovl204 mode2 | ovl203 mode1 |
|---|---|---|---|
| loader function | `FUN_8004F508` | `0x8004F964` | `0x8004FE84` |
| object pointer global | `0x8019F51C` | `0x801AB67C` | `0x8019B484` |
| stream/work pointer global | `0x8019F520` (initial `0x800B17C8`) | `0x801AB680` (initial `0x800BD960`) | `0x8019B488` (initial `0x800BD730`) |
| stage byte (local copy) | `0x8019F518` | `0x801AB678` | `0x8019B480` |
| stage byte written at | `0x8004F58C` ← `0x800411F8` | `0x8004F9F0` ← `0x800411F8` or `0x80041594` | `0x8004FEF8` ← `0x800411F8` or `0x80041594` |
| object load (`FUN_800165E4(2, S<<1, base)`) | `0x8004F77C..0x8004F784` → `0x801C4B40` | `0x8004FB74..0x8004FB7C` → `0x801C8460` | `0x8005007C..0x80050084` → `0x801B80C8` |
| stream load (`FUN_800165E4(2, S<<1|1, *streamptr)`) | `0x8004F79C` | `0x8004FBA0` | `0x800500A8` |
| INIT call `[*objptr+0x14]` | `0x8004F9F0` | `0x8004FD00` | `0x80050208` |
| per-frame TICK `[*objptr+0x00]` | `0x8008AB80` (driver) | `0x8004C190` | `0x8004C2F4` |
| stage-indexed u16 table | — | `0x8009C1E8` read at `0x80081FDC` | `0x8009C3BC` read at `0x80082248` |

**Who writes the stage bytes.** The mode overlays write their own copies
(`0x801AB678`, `0x8019B480`) in their loaders, from one of two GameState bytes in the
exe's bss: bit 0 of `0x80049D2A` selects `0x80041594` (set) or `0x800411F8` (clear)
(ovl204 `0x8004F994..0x8004F9F0`, ovl203 `0x8004FEC4..0x8004FEF8`). ovl202 always uses
`0x800411F8` (the mission number, confirmed live = 30 in "Eliminate Squatters",
`MISSION_SYSTEM.md`). The GameState bytes themselves are stored only by the front end,
ovl201: `0x800411F8` at `0x800714B8` (=70), `0x8008A890`, `0x800A4224` (=0),
`0x800A5E24`, `0x800AABE8`; `0x80041594` at `0x800A8060` and `0x800AABF0`;
`0x80049D2A` cleared at `0x800AABE0`. The exe and ovl202–204 contain no store to any of
the three (scan of all five disassemblies for `lui`/lo16 pairs resolving to them). So
the arena/versus stage number (0..4 → entries 160..168 / 180..188, i.e. "missions"
80..84 and 90..94 in `type_catalog_long.csv`) is chosen in the front end and carried in
`0x80041594` (INFERRED from the store sites; which menu writes which value is open).

---

## d. Mission number → FDAT entry pair → name

Mission number `N` = GameState byte `0x800411F8`; object = entry `2N`, chunk stream =
entry `2N+1` (`MISSION_SYSTEM.md`, confirmed live for N=30 and N=33). Names are the
`>`-terminated strings at offset `0x100 + N*0x20` of `GG/MS/MIS.T` entry 0
([MIS_FORMAT.md](MIS_FORMAT.md)), read here directly from disc LBA 101920+1. The MIS.T
table has 51 slots (0..50); the three `"NN Omitted"` slots are exactly the three even
entries missing from FDAT_T (32, 58, 78), which confirms the indexing.

| N | entries | MIS.T name | notes |
|---|---|---|---|
| 0 | 0 / 1 | Dummy00 | intro/training object; `ENTITY_TYPES.md` M0 |
| 1 | 2 / 3 | Stop Terrorist Threat | M1 in `ENTITY_TYPES.md` |
| 2 | 4 / 5 | Remove Gun Emplacement | M2 |
| 3 | 6 / 7 | Rescue Survey Team | M3 |
| 4 | 8 / 9 | Terrorist Pursuit | |
| 5 | 10 / 11 | Worker Robot Removal | |
| 6 | 12 / 13 | Secret Factory Recon | |
| 7 | 14 / 15 | Exterminate Organisms | |
| 8 | 16 / 17 | Guard Freight Train | |
| 9 | 18 / 19 | Destroy Fuel Depot | |
| 10 | 20 / 21 | Prototype MT Test | no chunk-12 spawn table (`ENTITY_TYPES.md`) |
| 11 | 22 / 23 | Guard Airplane | |
| 12 | 24 / 25 | Stop Gas Exposure | no chunk-12 |
| 13 | 26 / 27 | Prototype MT Test | |
| 14 | 28 / 29 | Repulse Enemy Attack | no chunk-12 |
| 15 | 30 / 31 | Exterminate Organisms | |
| 16 | 32 / 33 | *16 Omitted* | no files on disc |
| 17 | 34 / 35 | Guard Wharf Warehouse | |
| 18 | 36 / 37 | Remove Base Occupants | |
| 19 | 38 / 39 | Destroy Space Catapult | |
| 20 | 40 / 41 | Destroy Base Generator | |
| 21 | 42 / 43 | Mop Up Chrome Remnants | |
| 22 | 44 / 45 | Destroy "Justice" | |
| 23 | 46 / 47 | Chrome Uprising | |
| 24 | 48 / 49 | Destroy Plus Escapee | |
| 25 | 50 / 51 | Destroy Intruders | |
| 26 | 52 / 53 | Destroy Plane Computer | |
| 27 | 54 / 55 | AC Battle | |
| 28 | 56 / 57 | Attack Urban Center | |
| 29 | 58 / 59 | *29 Omitted* | no files on disc |
| 30 | 60 / 61 | Eliminate Squatters | confirmed live (`MISSION_SYSTEM.md`) |
| 31 | 62 / 63 | Eliminate Squatters | same name, different object; spawn table identical to 61 |
| 32 | 64 / 65 | Destroy Unknown MTs | |
| 33 | 66 / 67 | Rescue Transport Truck | confirmed live (`mission_load_sequence.md`) |
| 34 | 68 / 69 | Eliminate Strikers | |
| 35 | 70 / 71 | Stop Security MTs | |
| 36 | 72 / 73 | Stop Gang, "Dark Soul" | |
| 37 | 74 / 75 | Reclaim Oil Facility | |
| 38 | 76 / 77 | Recover Capsules | |
| 39 | 78 / 79 | *39 Omitted* | no files on disc |
| 40 | 80 / 81 | Guard Factory Entrance | |
| 41 | 82 / 83 | Capture Space Station | |
| 42 | 84 / 85 | Release Organisms | |
| 43 | 86 / 87 | Retake Air Cleaner | |
| 44 | 88 / 89 | Kill "Struggle" Leader | |
| 45 | 90 / 91 | Stop Security MT | |
| 46 | 92 / 93 | Destroy Base Computer | |
| 47 | 94 / 95 | Mop Up Chrome Remnants | |
| 48 | 96 / 97 | Destroy Floating Mines | |
| 49 | 98 / 99 | Destruction of Ravens | |
| 50 | 100 / 101 | AC Battle | name exists, **no entries 100/101 on disc** |
| 60 | 120 / 121 | **unknown** | outside the 51-slot name table; ovl202 base; INIT table has one type |
| 70 | 140 / 141 | **unknown** | outside the name table; ovl202 base |
| 80..84 | 160..168 / 161..169 | — | mode2 arena stages 0..4 (stage byte from `0x80041594`/`0x800411F8`); 168 ≡ 166 |
| 90..94 | 180..188 / 181..189 | — | mode1 versus stages 0..4; 188 is the empty stub |

Which front-end screen sets N=60/70 (entries 120/140 — plausibly the garage "Test AC"
arena and/or the intro, HYPOTHESIS) is open; `LIVE_CHECKLIST.md` MIS-11 covers the
same question for the PA byte. Entry 200 is the shared stream loaded before every
object and is not a code unit (no pointer block, no prologues).

---

## e. Per-unit table (from `tools/loop/functions.tsv`, `inventory.py --missions`)

Text ranges here are the detected ones (CODE_UNITS.tsv is not in this checkout); they
agree with CODE_UNITS.tsv for 57 of 58 units. The exception is fdat074, whose
CODE_UNITS `text_va` is `0x801C4D48`, 8 bytes before the first prologue; the two words
there (`27100007 00000010`) are not instructions, so the detected `0x801C4D50` is used.
`jt` = functions with a switch jump table (all tables sit in the rodata block between
the header and `.text`, `0x801C4C60..`); no mission function uses the GTE.

| unit | base | text | functions | largest | jt |
|---|---|---|---|---|---|
| fdat000 | 0x801C4B40 | 0x801C52C0..0x801CAB98 | 47 | 4912 (`func_801C981C`) | 6 |
| fdat002 | 0x801C4B40 | 0x801C4DD8..0x801C6CF0 | 32 | 740 (`func_801C529C`) | 4 |
| fdat004 | 0x801C4B40 | 0x801C4D30..0x801C7060 | 31 | 1540 (`func_801C5EA8`) | 2 |
| fdat006 | 0x801C4B40 | 0x801C4CD8..0x801C61AC | 29 | 704 (`func_801C55D8`) | 2 |
| fdat008 | 0x801C4B40 | 0x801C5418..0x801CBEAC | 70 | 3468 (`func_801CB03C`) | 10 |
| fdat010 | 0x801C4B40 | 0x801C4C94..0x801C65DC | 30 | 1240 (`func_801C5FD4`) | 2 |
| fdat012 | 0x801C4B40 | 0x801C4DA0..0x801C7BD4 | 43 | 1540 (`func_801C6454`) | 3 |
| fdat014 | 0x801C4B40 | 0x801C4CB0..0x801C8478 | 63 | 1240 (`func_801C7A70`) | 3 |
| fdat016 | 0x801C4B40 | 0x801C5384..0x801CDC20 | 84 | 4912 (`func_801CC8A4`) | 8 |
| fdat018 | 0x801C4B40 | 0x801C4E14..0x801C80D8 | 56 | 796 (`func_801C7C8C`) | 5 |
| fdat020 | 0x801C4B40 | 0x801C52C0..0x801CAA3C | 45 | 4912 (`func_801C96C0`) | 6 |
| fdat022 | 0x801C4B40 | 0x801C4CFC..0x801C7468 | 38 | 1556 (`func_801C60B0`) | 4 |
| fdat024 | 0x801C4B40 | 0x801C4D04..0x801C7020 | 43 | 796 (`func_801C68BC`) | 3 |
| fdat026 | 0x801C4B40 | 0x801C52C0..0x801CAB88 | 47 | 4912 (`func_801C980C`) | 6 |
| fdat028 | 0x801C4B40 | 0x801C4CC0..0x801C73E4 | 38 | 1240 (`func_801C6568`) | 3 |
| fdat030 | 0x801C4B40 | 0x801C4C94..0x801C7AE8 | 54 | 920 (`func_801C64CC`) | 2 |
| fdat034 | 0x801C4B40 | 0x801C52FC..0x801CC4EC | 73 | 4912 (`func_801CB170`) | 7 |
| fdat036 | 0x801C4B40 | 0x801C4DBC..0x801C729C | 42 | 704 (`func_801C6860`) | 6 |
| fdat038 | 0x801C4B40 | 0x801C4CA0..0x801C6438 | 27 | 796 (`func_801C5EB4`) | 2 |
| fdat040 | 0x801C4B40 | 0x801C52C0..0x801CC084 | 74 | 4912 (`func_801CAD08`) | 6 |
| fdat042 | 0x801C4B40 | 0x801C54CC..0x801CCEF4 | 76 | 4912 (`func_801CBB78`) | 11 |
| fdat044 | 0x801C4B40 | 0x801C531C..0x801CC4B0 | 65 | 4912 (`func_801CB134`) | 8 |
| fdat046 | 0x801C4B40 | 0x801C52C0..0x801CB1E8 | 54 | 4912 (`func_801C9E6C`) | 6 |
| fdat048 | 0x801C4B40 | 0x801C52C0..0x801CB524 | 58 | 4912 (`func_801CA1A8`) | 6 |
| fdat050 | 0x801C4B40 | 0x801C5454..0x801CCD2C | 77 | 4912 (`func_801CB9B0`) | 11 |
| fdat052 | 0x801C4B40 | 0x801C5918..0x801CD8E8 | 68 | 4912 (`func_801CB678`) | 10 |
| fdat054 | 0x801C4B40 | 0x801C52A0..0x801CA4D8 | 49 | 3468 (`func_801C9668`) | 6 |
| fdat056 | 0x801C4B40 | 0x801C5438..0x801CD128 | 77 | 4912 (`func_801CBDAC`) | 10 |
| fdat060 | 0x801C4B40 | 0x801C5458..0x801CC2D8 | 67 | 4912 (`func_801CAF5C`) | 11 |
| fdat062 | 0x801C4B40 | 0x801C5438..0x801CBDF8 | 63 | 4912 (`func_801CAA7C`) | 10 |
| fdat064 | 0x801C4B40 | 0x801C4DF4..0x801C7030 | 31 | 1240 (`func_801C6A2C`) | 5 |
| fdat066 | 0x801C4B40 | 0x801C52C0..0x801CBAF4 | 62 | 4912 (`func_801CA778`) | 6 |
| fdat068 | 0x801C4B40 | 0x801C52E0..0x801CB9E0 | 63 | 4912 (`func_801CA664`) | 7 |
| fdat070 | 0x801C4B40 | 0x801C4CA0..0x801C5800 | 13 | 600 (`func_801C53E0`) | 2 |
| fdat072 | 0x801C4B40 | 0x801C4D28..0x801C6A5C | 24 | 1540 (`func_801C5600`) | 2 |
| fdat074 | 0x801C4B40 | 0x801C4D50..0x801C727C | 32 | 1000 (`func_801C4D50`) | 3 |
| fdat076 | 0x801C4B40 | 0x801C52C0..0x801CAF1C | 55 | 4912 (`func_801C9BA0`) | 6 |
| fdat080 | 0x801C4B40 | 0x801C52FC..0x801CC14C | 71 | 4912 (`func_801CADD0`) | 7 |
| fdat082 | 0x801C4B40 | 0x801C5438..0x801CCA8C | 77 | 4912 (`func_801CB710`) | 10 |
| fdat084 | 0x801C4B40 | 0x801C4D14..0x801C786C | 55 | 704 (`func_801C6B64`) | 6 |
| fdat086 | 0x801C4B40 | 0x801C5438..0x801CCE84 | 80 | 4912 (`func_801CBB08`) | 10 |
| fdat088 | 0x801C4B40 | 0x801C52DC..0x801CBF1C | 63 | 4912 (`func_801CABA0`) | 7 |
| fdat090 | 0x801C4B40 | 0x801C5384..0x801CCB98 | 80 | 4912 (`func_801CB81C`) | 10 |
| fdat092 | 0x801C4B40 | 0x801C5308..0x801CC0C0 | 73 | 4912 (`func_801CAD44`) | 8 |
| fdat094 | 0x801C4B40 | 0x801C52C0..0x801CB45C | 58 | 4912 (`func_801CA0E0`) | 6 |
| fdat096 | 0x801C4B40 | 0x801C52C0..0x801CC098 | 68 | 4912 (`func_801CAD1C`) | 6 |
| fdat098 | 0x801C4B40 | 0x801C52C0..0x801CCDF8 | 85 | 4912 (`func_801CBA7C`) | 6 |
| fdat120 | 0x801C4B40 | 0x801C52C0..0x801CA9B4 | 43 | 4912 (`func_801C9638`) | 6 |
| fdat140 | 0x801C4B40 | 0x801C52C0..0x801CAB50 | 45 | 4912 (`func_801C97D4`) | 6 |
| fdat160 | 0x801C8460 | 0x801C8580..0x801C8F44 | 10 | 556 (`func_801C86B8`) | 0 |
| fdat162 | 0x801C8460 | 0x801C8580..0x801C8D20 | 9 | 704 (`func_801C8798`) | 0 |
| fdat164 | 0x801C8460 | 0x801C8580..0x801C8E3C | 8 | 628 (`func_801C8580`) | 0 |
| fdat166 | 0x801C8460 | 0x801C8580..0x801C8808 | 4 | 332 (`func_801C8580`) | 0 |
| fdat168 | 0x801C8460 | = fdat166 | (skipped: byte-identical) | | |
| fdat180 | 0x801B80C8 | 0x801B81E8..0x801B86F8 | 11 | 332 (`func_801B8434`) | 0 |
| fdat182 | 0x801B80C8 | 0x801B81E8..0x801B89A8 | 12 | 600 (`func_801B81E8`) | 0 |
| fdat184 | 0x801B80C8 | 0x801B81E8..0x801B874C | 8 | 600 (`func_801B81E8`) | 0 |
| fdat186 | 0x801B80C8 | 0x801B81E8..0x801B8818 | 8 | 600 (`func_801B81E8`) | 0 |
| fdat188 | 0x801B80C8 | 0x801B81E8..0x801B81F0 | 1 | 8 (`func_801B81E8`, `jr ra; nop`) | 0 |

Total: 2769 functions in 58 units (fdat168 skipped). Regenerate with
`python tools/loop/inventory.py --missions`.

---

## f. The "stray" pointer `0x801D8CBC` in M0  (CONFIRMED: not stray)

`ENTITY_TYPES.md` lists M0 type 1's installed think as `0x801D8CBC`, "via installer
`801CAB4C`", and notes it lies outside entry 0. The installer is:

```
801CAB4C  addiu sp,sp,-24
801CAB50  lui   v0,0x801d
801CAB54  addiu v0,v0,-26596      ; 0x801D0000 - 0x67E4 = 0x801C981C  -> +0x50
801CAB5C  sw    v0,80(a0)
801CAB60  lui   v0,0x801c
801CAB64  addiu v0,v0,30140       ; 0x801C75BC                         -> +0x54
801CAB68  sw    v0,84(a0)
801CAB6C  lui   v0,0x801d
801CAB70  addiu v0,v0,-29508      ; 0x801D0000 - 0x7344 = 0x801C8CBC  -> +0x58 (think)
801CAB74  sw    v0,88(a0)
801CAB78  lui   v0,0x801d
801CAB7C  addiu v0,v0,-28212      ; 0x801C91CC                         -> +0x5C
801CAB80  jal   0x801c9518
801CAB84  sw    v0,92(a0)
```

`addiu` sign-extends its immediate, so GCC emits `lui 0x801D` for any address whose
low half is ≥ 0x8000. `0x801D8CBC` is `(0x801D << 16) | 0x8CBC` computed without the
sign extension; the value actually stored is **`0x801C8CBC`**, inside entry 0's `.text`
(`addiu sp,sp,-224` prologue, a 1296-byte function that also appears in 30 other
entries). The same artifact affects the other three pointers (`0x801D981C` →
`0x801C981C`, `0x801D91CC` → `0x801C91CC`) and M0's INIT data table (`0x801DAB98` →
`0x801CAB98`, the start of `.data`). Nothing at runtime lives at `0x801D8CBC` for this
purpose; `*0x8019F520` and the chunk-stream destination are unrelated (the live trace
shows the stream at `0x800B74D8`). `tools/mission/type_catalog.py` and any reader of
`type_catalog_long.csv` that composes `lui<<16 | lo16` need the sign-extended form.

---

## g. What a matcher needs for a mission-overlay function

- **Oracle flags.** `python tools/match/match_fn.py <vaddr> <file.c> --bin
  disc_map/extracted/FDAT_T/entries/NNN.bin --base <group base> --expect-size <size>`.
  No `--ovl` (that flag selects a program overlay at `0x8004ADA0`). The base is per
  group: `0x801C4B40`, `0x801C8460`, `0x801B80C8` (`inventory.MISSION_ENTRIES[NNN]`).
- **No markup names.** `ac1_markup.tsv` has 0 rows in `0x801B8000..0x80200000`, so every
  symbol the function touches is `func_<ADDR>` / `D_<ADDR>` and needs
  `--defsym NAME=0x<ADDR>`. That includes (1) the mission's own sibling functions and
  data (`D_801CAB98` style), (2) the program-overlay functions it `jal`s
  (`func_8008A80C` etc. — these are ovl202/203/204 addresses, resolvable only by
  `--defsym`, since the oracle has no symbol table for the overlay group), and (3) the
  exe functions (`func_800165E4`, markup names *do* exist for exe PsyQ calls and
  resolve automatically). Compute `--defsym` values with sign extension (section f).
- **`--rodata`.** Switch tables sit in the rodata block between the header and
  `.text` (`0x801C4C60` upward in every ovl202-group entry); `functions.tsv` `jt_addrs`
  lists them per function. The 16x/18x groups have none.
- **Manifest row shape** (`re/match/manifest.tsv`, tab-separated `vaddr  unit  source
  size  extra`): the unit column carries `fdatNNN` instead of a program-overlay number,
  the `extra` column carries `--base 0x801C4B40 --defsym … [--rodata …]`, and
  `check_all.sh` derives `--bin disc_map/extracted/FDAT_T/entries/NNN.bin` from the unit
  (the current `check_all.sh` passes `--ovl "$ovl"` unconditionally, so it needs a
  `fdat*` branch before the first mission row lands — driver/lead item). The C file
  goes to `re/match/fdatNNN/func_<ADDR>.c`.
- **Reuse.** Because the shared library is linked into ~30 entries, a matched C file
  for one copy should be re-tried against every twin (same size, masked-identical
  bytes) with only the `--defsym` values changed; `inventory.py`'s `size` column plus a
  masked hash is enough to find the twins.
- **Where to start.** `fdat060` (mission 30, confirmed live) has 67 functions; the
  3–5-instruction leaves `rank.py --unit fdat060` lists first are the cheapest proof of
  the path. The `jr ra; nop` stubs (fdat188's only function, the empty slots in every
  entry) are real but below `rank.py`'s default 3-instruction floor.

---

## Follow-ups (listed, not fixed)

- `tools/mission/type_catalog.py` (`OBJOBJ_BASE = 0x801C4B40` for all N, `MAX_MISSION
  = 95`) and `tools/mission/mission_parse.py` assume one base, so entries 160..188
  ("missions" 80..94) get no `objobj_init` rows; they also print `lui<<16|lo16` pointers
  without sign extension (section f).
- `ENTITY_TYPES.md`: the `+0x120 AUX_TBL` row and the M0 think `0x801D8CBC` should be
  corrected per sections b and f; `MISSION_SYSTEM.md`: "relocatable" → absolute.
- Who sets `0x80041594` / `0x80049D2A` bit 0 in the front end (which screen selects the
  arena/versus stage), and which screen sets N=60/70.
- QUERY's two output words and START's role in the entries where it is not the stub.
- `re/match/check_all.sh` needs the `fdat*` unit branch described in section g.
