# AC1 Mission -> PA stage file + spawn block-index -> geometry resolution

Target **SLUS-01323 (v1.1)**. Companion to `docs/MISSION_SYSTEM.md` (mission
runtime), `docs/PA_HEADER.md` (slot binding), `docs/PA_FORMAT.md` (container) and
`REFERENCE.md` §11/§12. All addresses in the **mission overlay = FDAT.T entry 202
(0xCA)**, base `0x8004ADA0` (extract `tools/extract/extract_t.py`, disassemble with
`mipsel-linux-gnu-objdump … --adjust-vma=0x8004ADA0`). Offsets byte-verified on the
**pristine** bin. Tool: `tools/mission/mission_stages.py` (reuses
`tools/mission_parse.walk_chunks`).

`CONFIRMED` = disasm/byte-verified here. `HYPOTHESIS` = inferred, not ground-truthed.

---

## Part 2 first (fully solved) — spawn block-index -> geometry  CONFIRMED

This is the more important result for rendering, and it is closed.

### The binding chain (CONFIRMED addresses)
A chunk-12 spawn record (256 × 40 B, `docs/MISSION_SYSTEM.md` §2) is copied into the
**instance template table `0x8019FAB8`** (256 × 44) by **`FUN_80073B74`** (`0x80073B74`):
per record it does `rec[+0]=0`, copies the 20 source halfwords to `rec[+0x04+2i]`
(`i=0..19`), then `rec[+2] = rec[+0x1A]`. So **source halfword `hwK` lands at
`rec[+0x04 + 2K]`** (source byte `2K`).

**`FUN_80078B14`** (`0x80078B14`) binds an instance template to display geometry:
```
lh   v1, 0x0A(a1)          ; v1 = a1[+0x0A] = template field 0x0A = source hw3 (byte 0x06)
sll/addu/sll/subu/sll …    ; v0 = v1 * 44
addu v0, v0, a2            ; a2 = 0x8019F538  ->  v0 = 0x8019F538 + hw3*44  (= blkrec)
lhu  …  0(v0)/2(v0)/4(v0)  ; blkrec[+0],[+2],[+4]  -> display struct counts
lw   v0, 0x28(v0)          ; blkrec[+0x28] = geometry work-RAM ptr
sw   v0, 0x80(a0)          ; -> display[+0x80]  (the drawn model)
```
So the **block index is spawn `hw3`** (source byte 0x06, signed int16; `-1` = none),
indexing the **44-byte record table at `0x8019F538`**; the geometry pointer is
`blkrec[+0x28]`. (Confirms `docs/MISSION_SYSTEM.md`/`REFERENCE.md` §12: `hw3` = block
index. NB: a different note that called it `hw5` is off by two halfwords — byte
offset 0x06 = `hw3`.)

### Where the records of `0x8019F538` come from  CONFIRMED
There are **exactly two writers** of `0x8019F538` in the whole overlay (only two
instructions reference offset `-2760` off `lui 0x801a`):

| record | writer | source |
| --- | --- | --- |
| **[0]** | PA loader `FUN_8004F1A8` @`0x8004F264` → registrar `FUN_80073AD8` (`record=0x8019F538+0`) | PA **entry 1** (placement directory) of the stage PA file |
| **[1..N]** | **chunk-11 handler `FUN_800739AC`** (`0x800739AC`) | **chunk 11** of the mission stream (FDAT entry 2N+1) |

`FUN_800739AC` (the open question in the objective) walks chunk-11: `count = u32[0]`,
then per record allocates `sublen-36` bytes via the mxt allocator (`0x8002A480`,
ptr `DAT_801A5DB8`) and writes a 44-byte record starting at **`0x8019F564`**
(`= 0x8019F538 + 44 = record[1]`), advancing the write pointer by 44 each iteration:
`rec[+0x28] = work-RAM ptr`, `rec[+0] = u16[src+40]>>2`, `rec[+2] = u16[src+42]>>2`.
This is the **same 44-byte record layout** the PA registrar writes for record [0].

So records 1..N are **per-mission geometry blocks embedded in the mission chunk
stream itself** (not in the PA file). The PA file supplies the static stage geometry
(record [0] = its entry-1 directory); the mission stream supplies mission-specific
models (parked enemies, scenario props) as chunk-11 records [1..N].

### Resolution algorithm (what a tool/renderer implements)
Given spawn `hw3`:
- `hw3 == -1` → no bound geometry (object drawn by its type/other path).
- `hw3 == 0`  → record[0] = the stage **PA file entry 1** placement directory.
- `hw3 == k>0` → record[k] = **chunk-11 sub-record `k-1`** of FDAT entry 2N+1.

### Verification  CONFIRMED on missions 0,1,2,3
Chunk-11 record count == max `hw3` seen in chunk-12 spawns, every mission tested:

| mission | chunk-11 recs (sub-lens) | max spawn hw3 | match |
| --- | --- | --- | --- |
| 0 | 1 (`[1324]`) | 1 | yes |
| 1 | 3 (`[692,652,692]`) | 3 | yes |
| 2 | 3 (`[140,68,104]`) | 3 | yes |
| 3 | 1 (`[140]`) | 1 | yes |

`hw3` also clusters by object role (e.g. mission 1: `0`=large static structures,
`1`=mobile enemies, `2`/`3`=prop groups) — consistent with “record [0] = stage,
records [1..N] = mission models”.

Run: `tools/mission/mission_stages.py --fdat <entries-dir> --mission N`.

---

## Part 1 — mission -> PA stage file

### PA-file selection from the stage byte  CONFIRMED (the decode)
The PA loader **`FUN_8004F1A8`** (`0x8004F1A8`) reads one GameState byte
`DAT_8004121B` and formats the path template `"P0\PA00.T"` (`0x8008D928`) with the
`0xCCCCCCCD` divide magic (`multu`/`mfhi`):
```
hi = (byte * 0xCCCCCCCD) >> 32
phase  digit  = hi >> 4   == byte // 20      ; selects P0/P1/P2/P3
tens   digit  = hi >> 3   == byte // 10
ones   digit  = byte - (byte//10)*10 == byte % 10
path = "P{byte//20}\PA{tens}{ones}.T"
```
**Therefore the PA FILE NUMBER == the stage byte itself**, and the directory is
`P{byte//20}`. Matches the on-disc layout (P0=PA00-19, P1=PA20-39, P2=PA40-59,
P3=PA60-71). The PA file loads into **file slot 0** (`load_T_file(0,path)`).

Examples (CONFIRMED decode): byte `0`→`GG/P0/PA00.T`, byte `23`→`GG/P1/PA23.T`,
byte `71`→`GG/P3/PA71.T`.

### Sub-resource PA entries  CONFIRMED (the decode)
Eight more GameState bytes `DAT_8004121D..0x1224` each conditionally load an
**extra `.T` entry from the same PA file** (file slot 0) as `entry = byte + addend`,
via `FUN_800537FC` (`255`/`0xFF` = skip). Addends read straight from the disasm:

| byte | addend | byte | addend |
| --- | --- | --- | --- |
| `0x8004121D` | +0x02 | `0x80041221` | +0x3C |
| `0x8004121E` | +0x0A | `0x80041222` | +0x46 |
| `0x8004121F` | +0x2C | `0x80041223` | +0x6C |
| `0x80041220` | +0x58 | `0x80041224` | +0x8E |

(These map to PA loader work slots `224..231` in `0x801A5F34` and are walked/relocated
just like the main geometry; they are extra geometry/texture banks for the stage.)

### Where the per-mission stage byte VALUE is set  OPEN / HYPOTHESIS
The decode above is fully confirmed, but the **per-mission value of `DAT_8004121B`
(and the 8 sub-resource bytes) is NOT stored statically** in either binary:

- In **FDAT entry 202** (the in-mission overlay) the stage byte `0x8004121B` and the
  sub-resource bytes `0x8004121D..0x1224` are **only ever read, never written**
  (exhaustive scan: no `sb/sh/sw`, no pointer-relative store, no block copy into the
  `0x80041200` GameState region writes those offsets — only `0x8004121A`,
  `0x80041226` get written, by `0x8007EE14`/`0x8007EDAC`).
- In the **base EXE** `SLUS_013.23` (text `0x80011000`, size `0x29000`) there is
  **no reference at all** to the `0x80041200` GameState block; the mission/phase byte
  `0x800411F8` is read (`0x8001253C`/`0x80012578`) but the stage block is untouched.
- `0x80041200..0x80041260` is **BSS** (above text end `0x8003A000`) → not shipped in
  either image; it is a runtime-populated GameState struct.

Conclusion (HYPOTHESIS): the stage byte is written into GameState by the **menu /
mission-launch path** (garage → “SORTIE”) before the in-mission overlay is invoked,
most plausibly from a per-mission descriptor (the FDAT 2N objective object’s init
method `[*0x8019F51C+0x14]` runs *before* the PA loader at `0x8004FA00`, but objective
object 0 was disassembled and does **not** write `0x121B`). The most likely real
source is base-EXE menu code outside the analysed text window, or a small per-mission
descriptor copied by that menu code. **This requires DuckStation ground-truth or a
deeper base-EXE menu trace to close** — it is NOT derivable from FDAT 2N/2N+1 alone.

### Practical mission -> PA table  (how to produce it)
Because the byte is set at runtime, `tools/mission/mission_stages.py` takes the stage byte
from `--stage BYTE` or a ground-truth `--stagemap` JSON (`{mission: byte}`) and then
prints the exact PA file(s) and the full block resolution. To build the verified
table: in DuckStation, start each mission, read `0x8004121B` (and `0x8004121D..0x1224`)
at the moment the PA loader runs, and append to the stagemap. With the byte known the
PA file and sub-resources are 100% determined by the confirmed decode above.

> Honest status: the **mechanism** mission→(stage byte)→PA file is CONFIRMED end to
> end; the **mission→byte map** is the single remaining unknown and needs runtime
> ground-truth (3+ readings) to populate the table. No fabricated PA numbers are
> listed here.

---

## Key addresses (all in FDAT entry 202, base `0x8004ADA0`)

| symbol | addr | role |
| --- | --- | --- |
| PA loader | `0x8004F1A8` | stage byte `0x8004121B` ÷20/÷10 → `P{n}\PA{nn}.T`, slot 0 |
| PA path template | `0x8008D928` | `"P0\PA00.T"` |
| sub-resource loader | `0x800537FC` | byte `0x121D..0x1224` + addend → extra PA entry |
| registrar (record[0]) | `0x80073AD8` | writes `0x8019F538+0`: `+0x28`=geom ptr |
| **chunk-11 handler (records[1..N])** | **`0x800739AC`** | writes `0x8019F564+i*44` from chunk 11 |
| instance table init (chunk 12) | `0x80073B74` | fills `0x8019FAB8` (256×44) from spawn recs |
| **per-object binding** | **`0x80078B14`** | `blkrec = 0x8019F538 + hw3*44`, geom = `blkrec[+0x28]` |
| 44-byte block record table | `0x8019F538` | [0]=PA entry-1 dir, [k]=chunk-11 rec k-1 |
| instance template table | `0x8019FAB8` | 256×44, src = chunk-12 spawn |
| stage byte (GameState) | `0x8004121B` | **= PA file number** (read-only here) |
| sub-resource bytes | `0x8004121D..0x1224` | extra PA entry selectors (read-only here) |
| mission/phase byte | `0x800411F8` | mission number (set by base) |

---

## CONFIRMED vs HYPOTHESIS

**CONFIRMED**
- PA file number == stage byte `0x8004121B`; dir = `P{byte//20}`; decode math at
  `0x8004F1A8`.
- 8 sub-resource bytes `0x121D..0x1224` → extra PA `.T` entries `byte+addend` (table
  above), same file slot 0.
- Spawn `hw3` (byte 0x06, −1=none) = block index → `0x8019F538 + hw3*44`, geom ptr
  `+0x28` (`FUN_80078B14`).
- Record[0] from PA entry-1 (registrar `FUN_80073AD8`); records[1..N] from **chunk 11**
  (`FUN_800739AC`) at `0x8019F564 + (k-1)*44`. Verified count==max-hw3 on missions
  0–3.

**HYPOTHESIS / OPEN**
- The per-mission **value** of `0x8004121B` (and the sub-resource bytes): set by
  runtime menu/mission-launch code, not in either binary statically — needs
  DuckStation ground-truth (or a base-EXE menu trace) to build the mission→PA table.
- `hw7` = object type id (CONFIRMED varies sensibly per mission) drives a separate
  type-based render/AI path for `hw3 == -1` objects; that path is not enumerated here.
