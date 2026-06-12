# AC1 Object / MT stats — where the per-type and per-instance numbers live

Target **SLUS-01323 (v1.1)**, pristine bin. All code addresses are in the
in-mission overlay = **FDAT.T entry 202 (0xCA)**, loaded to base **0x8004ADA0**
(extract `tools/extract/extract_t.py`; objdump
`mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL --adjust-vma=0x8004ADA0`).
Companion to `docs/MISSION_SYSTEM.md` §2 (spawn table). Tool: `tools/mission/object_stats.py`.

Notation: **CONFIRMED** = byte/disasm-verified here. **HYPOTHESIS** = inferred,
not yet ground-truthed in DuckStation.

---

## TL;DR — the headline result

There is **no single global "stats table keyed by type id"** with HP / damage /
fire-rate / name laid out as `base + type*stride`. I traced the spawner, the
instance->object binder, the per-frame object/collision loops, the script VMs and
every resource-table consumer in the overlay and found instead a **two-tier**
model:

1. **Per-INSTANCE numeric stats live in the chunk-12 spawn record itself**
   (halfwords `hw8..hw19`). These are the HP-/range-/area-like numbers a viewer
   wants, and they are written straight into the runtime instance template
   `0x8019FAB8` by the spawner `FUN_80073B74`. CONFIRMED that they are copied;
   their exact meaning is HYPOTHESIS (see field table below).

2. **Per-TYPE assets (model + behavior) come from a resource-SLOT system**, not a
   flat type array. The mission's chunk-0 stream carries up to **128 inline data
   blocks**; the scene loader registers their pointers into a **resource pointer
   table at `0x801A5F34`** (slot index x4). Each registered block is a typed pack
   (model geometry, instance descriptors, etc.) consumed by the render/AI code.
   The type id (`hw7`) selects which model/behavior block is used at bind time —
   it is **not** used as a direct array subscript into an HP/damage table.

So for a viewer: **read the spawn record's `hw8..hw19` for the displayable
per-instance numbers**, and treat `hw7` as the *type/class id* (a label, not a
table key). The player's own AC stats are a separate, fully-computed-from-parts
path (see "Player path" below) and are unrelated to enemy/object types.

---

## 1. The spawn record = the per-instance stats source  CONFIRMED (layout) / HYPOTHESIS (semantics)

Chunk 12 of the `2N+1` mission stream = **256 records x 40 bytes** = 20 int16
halfwords each (`docs/MISSION_SYSTEM.md` §2). The spawner `FUN_80073B74`
(`0x80073B74`) zero-inits a 44-byte instance record at `0x8019FAB8 + i*44`, copies
all 20 source halfwords to instance `+0x04..+0x2B`, then sets instance `+0x00 =
instance +0x18` (a derived field). So every halfword survives into the live
instance template and is therefore readable/displayable.

| src hw | byte | instance off | meaning | status |
| --- | --- | --- | --- | --- |
| hw0,hw1,hw2 | 0,2,4 | +0x04..+0x08 | **X, Y, Z** world position | CONFIRMED |
| hw3 | 6 | +0x0A | **geometry block index** (-1 = none); binds to PA block via `0x8019F538` | CONFIRMED |
| hw4 | 8 | +0x0C | 0 in all observed records | HYPOTHESIS (unused/flags) |
| hw5 | 10 | +0x0E | **rotation** (PSX angle units; 1024/2048/3072 = 1/4 1/2 3/4 of 4096) | CONFIRMED (range) |
| hw6 | 12 | +0x10 | 0 observed | HYPOTHESIS |
| **hw7** | 14 | **+0x12** | **object / MT TYPE id** (read as a byte at +0x12 by some paths) | CONFIRMED |
| hw8 | 16 | +0x14 | per-instance param A — small (0,32,64,288,512,3616); **AI/aggression or activation flags** | HYPOTHESIS |
| hw9 | 18 | +0x16 | per-instance param B (0 in all observed) | HYPOTHESIS |
| hw10 | 20 | +0x18 | copied to instance +0x00 (derived/link) | HYPOTHESIS |
| hw11 | 22 | +0x1A | **per-instance "big" value** — 1300/2050/4500/5200/6000/9800; **candidate HP / armor / detection-range** | HYPOTHESIS |
| hw12 | 24 | +0x1C | per-instance value — 110/130/300/313/480/2500; **candidate sub-stat (range / damage / area-id)** | HYPOTHESIS |
| hw13 | 26 | +0x1E | 0/600 observed | HYPOTHESIS |
| hw14 | 28 | +0x20 | 0/2561 observed | HYPOTHESIS |
| hw15 | 30 | +0x22 | 0/513/3000/20000 observed; **candidate area/trigger id** | HYPOTHESIS |
| hw16..hw19 | 32..38 | +0x24..+0x2A | mostly 0; bulk-copied as words by the binder | HYPOTHESIS |

> Important observation: `hw11`/`hw12` are frequently **identical across many
> records of the same mission group** (e.g. mission 1 records 5-16 all share
> `hw11=2050, hw12=313`). That argues these are **group/area parameters**, not a
> per-unit HP. The records that *differ* (the "special"/boss entries) carry the
> large distinctive `hw11` (9800, 6000, 5200, 4500) — those are the best HP
> candidates and the right thing to ground-truth first.

### Instance->object binder `FUN_80078B14` (`0x80078B14`)  CONFIRMED
Copies the 44-byte instance template into a live 368-byte active object in the
pool **`0x801A26B8`** (16 slots x 0x170). It reads instance `+0x0A` (block, for the
PA `0x8019F538` 44-byte record), `+0x02/+0x04/+0x06/+0x08/+0x14`, and bulk-copies
`+0x0C..+0x2C`. It does **NOT** read `+0x12` (type) to index any stat table — the
type id is not a subscript here.

---

## 2. The resource-slot table `0x801A5F34` (per-TYPE assets)  CONFIRMED

The mission chunk stream's setup chunks (chunks 0-2, handler `FUN_80053848`
`0x80053848`) carry **inline data blocks**: chunk 0 starts with a u32 **count**
(observed 128), then each block is `[u32 len][...][data@+8]`. `FUN_80053848`
registers each block's pointer into a pointer table:

```
0x801A5F34[slot] = &block      (slot = running block index, bounded < 5 in some paths)
FUN_800574D8(block + block[+8]) ; per-block unpack/relocate
```

The generic loader `FUN_8005360C` (`0x8005360C`) does the same for blocks pulled
from FDAT via `read_T_entry`, storing the dest pointer at `0x801A5F34[a3]`.

**Consumers** (CONFIRMED these read `0x801A5F34`):
- `FUN_80054750` / `FUN_80055A40` (`0x80055A40`): treat **slot 0** as a model pack.
  Header has a **count byte at `+0x05`** and an **offset word at `+0x08`** to a
  **28-byte-record array**; record `idx` is at `block + block[+8] + idx*0x1C + 12`,
  and `+0x18` of the record is a sub-pointer. The consumer is the **GTE/3D render
  path** (`mtc2`/`lwc2`/`rtps`), i.e. these 28-byte records are **model/instance
  draw descriptors**, not HP/damage stats.
- `FUN_8004F4B8`, `FUN_800564DC`: read slot-0 header (`+0x04`, `+0x05`, `+0x08`)
  for setup/DMA — again model/asset, not stats.

So the per-type data reachable through `hw7` is **model + render + (per-mission)
behavior code**, sliced by resource slot, not a numeric stat array. A clean
HP/damage/name lookup of the form `statbase + hw7*stride` does **not** exist in the
entry-202 overlay. (NEGATIVE RESULT, deliberately reported.)

---

## 3. Player path (for contrast)  CONFIRMED

The player AC's stats ARE computed from a part table, but this is the **player**,
not enemy types: `FUN_8007EAE8` (`0x8007EAE8`) reads the equipped-part bytes at
`0x800411F9..0x80041201` and the player config at `0x80041204/0x80041210`, scaling
them by constants into the player stat block `0x800411A8..0x800411B6`. The garage /
sell loop `FUN_8008AFC0` iterates the equipped-parts array at **`0x80041278`**
(count byte `0x80041278`, **60-byte records**, value flag bit `0x10` at record
`+0x12`) and divides by 100 (`multu 0x51EB851F`) to compute money. These confirm
"part-stat tables exist" — but they are the **player's garage data at 0x800411xx**,
keyed by equipped-slot, and are unrelated to the mission object/MT TYPE ids.

---

## 4. Names / strings  OPEN

No object-type -> name string table was located in entry-202. Mission/enemy display
text lives in **MIS.T** (`docs/DISC_TEXT_INVENTORY.md`); whether any of those
strings are indexed by `hw7` was not established here. Object/MT display names are
therefore **OPEN**.

---

## CONFIRMED vs HYPOTHESIS summary

**CONFIRMED**
- Per-instance numbers are carried in the spawn record `hw8..hw19` and copied
  verbatim into the runtime instance template `0x8019FAB8` by `FUN_80073B74`.
- `hw7` = type/class id; binder `FUN_80078B14` does not use it as a stat subscript.
- Resource-slot pointer table `0x801A5F34` (filled by `FUN_80053848` /
  `FUN_8005360C`); slot 0 = a model pack (count@+5, table@+8, 28-byte render
  records) consumed by the GTE render path `FUN_80055A40`.
- No `statbase + hw7*stride` HP/damage/name table exists in entry-202 (negative).
- Player AC stats are a separate part-driven path (`FUN_8007EAE8`, data @0x800411xx).

**HYPOTHESIS / OPEN**
- Exact meaning of `hw8..hw19` — best HP candidate is `hw11` (the distinctive
  large value on "special" records); `hw11`/`hw12` look group/area-shared on
  ordinary records. **Ground-truth needed.**
- Object/MT display names (likely in MIS.T; index unknown).
- Whether a per-type numeric table lives in a *lower FDAT entry* loaded by the
  base EXE rather than the overlay (not searched here; out of overlay scope).

## Single best next step
In DuckStation, start mission 1, breakpoint `FUN_80078B14` (`0x80078B14`) and watch
the instance template `0x8019FAB8 + i*44` for a known unit (e.g. the type-5 record
with `hw11=9800`); then deal damage to it and **watch which instance/object
halfword decrements** — that pins HP to a concrete offset (test `hw11` @ +0x1A
first), and the same watch confirms whether `hw12`/`hw8` are range/AI vs HP.
