# AC1 (SLUS-01323) — MXT container & overlay loader

Reverse-engineered from the **resident exe** (Ghidra DB, `0x80011000–0x80039fff`
+ resident game loader `0x80015xxx–0x80016xxx`) and ground-truthed live in
DuckStation. This is the mechanism behind every `.T` file load, the FDAT overlay
system, and the trailing-checksum gotcha.

The game's own debug `printf`s name this system **MXT** (`mxt_head_work`,
`mxtid`, `tsu`). Each on-disc `.T` file is an **MXT archive**; `mxtid` = the
group/archive id; `tsu` = entry count.

## 1. The MXT registry (group table) — base `0x8004a2a4`, stride `0xc`

One 12-byte record per `mxtid` (archive). Built at boot by `mxt_register`:

| off | field            | notes                                            |
|-----|------------------|--------------------------------------------------|
| +0  | offset-table ptr | → into `mxt_head_work` arena (the entry index)   |
| +4  | base `CdlLOC`    | file start on disc (min,sec,frame BCD + 1 pad)   |
| +8  | `u32` size       | file size in bytes                               |

Live dump, confirmed:

| mxtid | file            | base CdlLOC | size        | offtbl ptr  |
|-------|-----------------|-------------|-------------|-------------|
| 1     | `COM\RTIM.T`    | 18:59:56    | 0x0103c800  | 0x80043508  |
| 2     | `COM\FDAT.T`    | 16:04:39    | 0x019ab000  | 0x800435da  |
| 4     | `MS\MENU_TIM.T` | 22:19:20    | 0x00172800  | 0x80043776  |
| 5     | `MS\MENU_TMD.T` | …           | …           | …           |
| 6     | `MS\MENU_VAB.T` | …           | …           | …           |
| 7–0x12| AC part data    | (see §4)    |             |             |
| 0x13  | `MS\MIS.T`      |             |             |             |

`COM\FDAT.T` (mxtid 2) base CdlLOC 16:04:39 → LBA `16*4500 + 4*75 + 39 = 72339`
(matches the long-known "FDAT sectors 72189+").

## 2. Archive header / entry index ("tsu" table)

`mxt_register` (`FUN_80016678`) reads the **first sector** (2048 B) of the `.T`:

```
sector[0]      = u16 count           (= "tsu", number of entries)
sector[1..]    = u16 offset[0..count]  (entry start, in SECTORS, file-relative)
```

It copies `count+1` u16 offsets into the shared `mxt_head_work` arena (ends
before `0x80048507`; overflow → `"ERROR!!! over mxt_head_work"`). The registry
`+0` pointer points at this copied table.

**Entry N occupies sectors `offset[N] .. offset[N+1]`** (length =
`offset[N+1]-offset[N]` sectors). `offset[count]` = file end.

FDAT.T (mxtid 2) live: 206 entries (0..205); `offset[205] = 0x3356 = 13142 =
size/2048` — table closes exactly. Sample:
`offset[0]=1, [1]=0x11, [3]=0x85, [4]=0x1bf …` (steady growth).

## 3. The loaders

### `mxt_load_entry(mxtid, entry, dest)` — `FUN_800165e4`
```c
tbl   = registry[mxtid].offtbl;          // u16*
start = tbl[entry];
nSect = tbl[entry+1] - start;            // length in sectors
loc   = CdIntToPos( registry[mxtid].baseLBA + start );   // FUN_80015a08
do {
    raw_read(loc, dest, nSect);          // FUN_80016328
} while ( mxt_verify(dest, nSect) );     // FUN_80015b24 — RE-READ until checksum OK
```

### raw sector read — `FUN_80016328(loc, buf, nSectors)`
`CdControl(CdlSetloc, loc)` → `CdRead(nSectors, buf, mode 0x80)` → `CdReadSync`,
3× retry on hardware error.

### LBA→pos — `FUN_800158c8` = `CdPosToInt`
`btoi(min)*4500 + btoi(sec)*75 + btoi(frame)` (4500 = 75*60).

## 4. Confirmed FDAT.T (mxtid 2) entry map

Boot driver `FUN_800122a8` loads, in order:
- `mxt_load_entry(2, 0xc9 /*201*/, work)` → **front-end / UI overlay**.
  Live: sectors 12475→12740 = **265 sectors ≈ 530 KB**. *(This is the
  "entry-201" the Farsi translation track patches.)*
- Then by mode selector `DAT_80039c5e`: entry **0xca/0xcb/0xcc (202/203/204)**
  — the three mode sub-overlays. Live sizes: 202 = 140 sectors (~280 KB),
  203/204 = 131 sectors each. **202 = the in-mission runtime overlay** (loads to
  `0x80050000`, NOT in the Ghidra DB — the DB holds the menu build).

AC garage part data lives in dedicated archives, one per slot category:
`CORE_T(7) LEG_T(8) ARMS_T(9) HEAD_T(10) BST_T(0xb) BWL_T(0xc) BWR_T(0xd)
WEL_T(0xe) WER_T(0xf) COMP_T(0x10) SPEC_T(0x11) GENE_T(0x12)`; missions in
`MIS.T(0x13)`.

## 5. THE CHECKSUM (root cause of the "NOW LOADING" hang)

`mxt_verify` (`FUN_80015b24`) — exactly the trailing-checksum scheme:

```c
bool mxt_verify(int* buf, int nSectors) {
    int sum = 0x12345678;                          // seed
    for (i = nSectors*0x200 - 2; i != -1; i--)     // every word except the last
        sum += *p++;                               // (nSectors*512 words/sector)
    return sum != buf[nSectors*0x200 - 1];         // last word = stored checksum
}
```

**Why a bad edit hangs at "NOW LOADING":** the loader is
`do { read; } while (mxt_verify != 0);`. A wrong checksum does not error — it
makes the game **re-read the same sectors from disc forever**. Any edit to an
MXT entry must recompute `seed 0x12345678 + sum(words) → last word`, or the boot
spins. This confirms (with the exact code + retry mechanism) the
`project_ac1_overlay_checksum` finding.

## 6. Code overlays: why they're absent from Ghidra, and how to get them

Entries 201–204 are **code overlays**, not data. They all load to the **same**
fixed RAM base `0x8004ada0` (= `PTR_DAT_800112c8`) with **no relocation**, so a
raw entry payload *is* the runtime memory image (minus its trailing checksum).
Only one overlay occupies those addresses at a time:

- The **Ghidra DB** captured the **entry-201 (front-end) build** of the region
  (its menu functions live at `~0x8009xxxx`). The mission/mode builds of the same
  addresses were never imported → in-mission functions are simply not in the DB.
- Proof, same address `0x800851a8`: mission build = `94 82 00 00` (`lhu`,
  round-think); menu build = `24 06 01 20`; Ghidra = "No function found".

### Overlay ABI (confirmed)
Each overlay image begins with a small header:

| off  | value (mission/202)      | meaning                                            |
|------|--------------------------|----------------------------------------------------|
| +0x00| `0x00000004`             | (flags/version — same across overlays)             |
| +0x04| `0x8004c340`             | **main-entry fn ptr** — boot calls `(*0x8004ada4)()` each frame |
| +0x08| `45 4e 45 52 47 59 00 00` | signature **`"ENERGY\0\0"`** (identical in all overlays) |

So `0x8004ada4` (= base+4) is the per-overlay main loop. Verified: the mission
entry `0x8004c340` is a real prologue (`addiu sp,sp,-72; …`). Main entries:
202=`0x8004c340`, 203=`0x8004bf94`, 204=`0x8004bf3c`. These are the first
functions to analyze in each imported overlay.

### Extraction — `tools/extract/extract_overlay.py`
Carves overlays out of `fdat_extracted.T` (or `--disc` from the pristine .bin)
using the count-first TOC, verifies the MXT checksum, writes
`disc_map/overlays/ovl<N>_<name>.bin`, and emits a Ghidra import script.

| entry | name       | sectors      | size      | checksum   | RAM range             | distinct? |
|-------|------------|--------------|-----------|------------|-----------------------|-----------|
| 201   | front_end  | 12475–12740  | 542,720 B | 0x62d3ec3c | 0x8004ada0–0x800cf5a0 | front-end (Ghidra DB build) |
| 202   | mission    | 12740–12880  | 286,720 B | 0x3ebcb5b3 | 0x8004ada0–0x80090da0 | **MISSION (sortie)** |
| 203   | mode1      | 12880–13011  | 268,288 B | 0xf9476ae5 | 0x8004ada0–0x8008c5a0 | **LINK-CABLE VERSUS** |
| 204   | mode2      | 13011–13142  | 268,288 B | 0xe6e8170c | 0x8004ada0–0x8008c5a0 | **LOCAL 2-AC ARENA BATTLE** |

All four checksums **validate** (seed `0x12345678`). 202/203/204 are only ~15%
byte-similar in the code region — three genuinely different programs, selected by
the front-end via `DAT_80039c5e` = 0/1/2.

### Mode identities (from strings + decompiled entry behaviour)
- **202 MISSION** — entry `0x8004c340` runs the mission driver `FUN_8008ab68`,
  counts down the mission timer `_DAT_8019f52c`, steps the objective object via
  its vtable `(*(_DAT_8019f51c+4))()`, and ends on success/fail flags
  `_DAT_8019f524 & 0x100/0x200` → result `_DAT_80048610` = 1/2. String: `MISSION TIMER`.
- **203 LINK-CABLE VERSUS** — strings `LINK CABLE ERROR`, `REMOTE:ERROR:FRAME`,
  `REMOTE:ERROR:RAND` (two-console frame-lockstep + shared-RNG sync), `ENEMY LIMIT`;
  loads the fixed arena stage `P0\PA00.T`. Entry `0x8004bf94`.
- **204 LOCAL 2-AC ARENA BATTLE** — entry `0x8004bf3c` copies **two** AC config
  blocks (`0x80040fa0`→`0x8008c2b8`, `0x8004133c`→`0x8008c654`) and instantiates
  two ACs (`FUN_8008156c` ×2 via `_DAT_80039d18`) on the same fixed PA00 arena,
  with **no** link networking — the single-console duel (attract/demo and/or vs-CPU).

### Validation
- Carved `ovl202_mission.bin` == prior `disc_map/fdat_entry202.bin` **exactly**.
- vs **live RAM** (in-mission, paused): 99.91% identical; **100% of code
  matches** (round-think bytes identical); the only diffs are the final ~16 KB
  (overlay BSS/scratch mutated at runtime). The static carve is a deterministic,
  checksum-valid stand-in for a live dump — better for analysis (no runtime mutation).

### Importing into Ghidra
Two ways:
- **GUI**: `ghidra_scripts/import_ovl_<N>_<name>.py` creates an **overlay** memory
  block at `0x8004ada0` from the carved bin (so resident calls `0x80010000–0x8003ffff`
  still resolve), then disassembles. Run from the Script Manager, Auto-Analyze.
- **Headless (no GUI) — `tools/ghidra/ghidra_overlay.sh`** *(recommended)*: imports +
  auto-analyzes a carved overlay once (cached project), then decompiles any
  function address with full Ghidra quality:
  ```
  tools/ghidra/ghidra_overlay.sh disc_map/overlays/ovl202_mission.bin 0x8008ab68 0x8004c340
  ```
  Verified: ovl202 auto-analyzes to **585 functions**; the mission main loop and
  driver decompile cleanly and confirm the live-RE mission lifecycle. This gives
  the combat/physics RE the xref-aware Ghidra decompiles the live DB can't (the DB
  only has the entry-201 build of these addresses). Helper scripts:
  `ghidra_scripts/OvlReport.java`, `ghidra_scripts/DecompAt.java`.

## Addresses (resident, Ghidra DB)
- `FUN_80016678`  mxt_register(mxtid, path)
- `FUN_800165e4`  mxt_load_entry(mxtid, entry, dest)
- `FUN_800165a8`  mxt_load_raw(dest, registryRec, len) — len==0 ⇒ use size field
- `FUN_80016328`  raw_read(loc, buf, nSectors)  [CdControl+CdRead+CdReadSync, 3× retry]
- `FUN_80015b24`  mxt_verify(buf, nSectors)  [checksum, seed 0x12345678]
- `FUN_80015a08`  baseLBA+offset → CdlLOC
- `FUN_800158c8`  CdPosToInt(CdlLOC)
- `FUN_800122a8`  boot/data-flow driver (registers all archives, mode overlay loop)
- `FUN_800121ac`  NOW-LOADING screen (loads RTIM.T mxtid1 entry 0x65)
- registry base `0x8004a2a4` (stride 0xc); `mxt_head_work` index arena ends <`0x80048507`
