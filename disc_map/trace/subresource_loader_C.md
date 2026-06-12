# Sub-trace C — sub-resource loader (live, 2026-06-11)

Caught live by a **read-watchpoint on `0x8004121D`** during a fresh stage load
("Eliminate Squatters"). The loader fired at PC `0x8004F2A8`.

## Overlay-aliasing note (important)
The loader code lives at `~0x8004F2A0..0x8004F4B0` and the entry-load helper at
`0x800537fc`. **These addresses hold *different* (gameplay) code while a mission is
running** — the load-phase overlay is only mapped during the load. So you can only
disassemble/breakpoint the loader by trapping it at load time (as here). The geometry
runtime addresses (0x80057xxx walker/emitter, 0x8019F538/0x8019FAB8 tables) are NOT
aliased and are valid in-mission.

## Algorithm (unrolled, 8 slots)
After the main PA file (stage byte `0x8004121B` → `P{n}\PA{nn}.T`) is loaded and
registered (`jal 0x80073ad8` registrar at 0x8004F270), the loader processes 8
sub-resource selector bytes `0x8004121D..0x8004121224`:

```
dest_ptr = 0x8019F520          ; running work-buffer pointer (advanced each load)
for i in 0..7:
    b = byte[0x121D + i]
    if b == 0xFF: continue                      ; 0xFF = "no sub-resource in this slot"
    entry = b + ADDEND[i]                        ; pick entry within a slot-category
    bytes = load_entry(*dest_ptr, 0, entry, 0x80)   ; fn 0x800537fc(dest,0,entry,0x80)
    *dest_ptr += bytes                           ; resource id passed on stack = 0xE0+i
```

## Per-slot addend table (CONFIRMED from the unrolled code)
| slot | byte @ | addend | entry base | resource id (stack) |
|------|--------|--------|-----------|---------------------|
| 0 | 0x121D | +2   | e2   (3-part blocks)      | 0xE0 |
| 1 | 0x121E | +10  | e10  (21-part AC/MT models)| 0xE1 |
| 2 | 0x121F | +44  | e44  (3-part objects)     | 0xE2 |
| 3 | 0x1220 | +88  | e88                       | 0xE3 |
| 4 | 0x1221 | +60  | e60  (6-part VARIABLE)    | 0xE4 |
| 5 | 0x1222 | +70  | e70                       | 0xE5 |
| 6 | 0x1223 | +108 | e108                      | 0xE6 |
| 7 | 0x1224 | +142 | e142                      | 0xE7 |

These bases line up **exactly** with the PA_SLOTS.md slot-category map (e2 / e10 / e44 /
e60 / e125-165 effects, etc.). So **each sub-resource byte selects *which* entry within
a category to pull into the stage** — i.e. the per-mission choice of MT model, object
set, effect bank, etc. The main PA file supplies the environment; the 8 selectors pick
the stage-specific object/MT/effect variants.

Cross-dependency: between slots 4 and 5 the code re-reads `byte[0x1220]` and, if it is in
`[8,15]`, **skips slot 5** (`0x8004F3CC..0x8004F3E0`).

## Worked example — "Eliminate Squatters"
Stage byte `0x121B = 0x00` → main = P0\PA00.T (same stage geometry as the training
mission — missions reuse stages). Sub-resource bytes `00 03 01 01 00 03 ff ff`:
| slot | byte | → entry |
|------|------|---------|
| 0 | 0x00 | e2 |
| 1 | 0x03 | e13 |
| 2 | 0x01 | e45 |
| 3 | 0x01 | e89 |
| 4 | 0x00 | e60 |
| 5 | 0x03 | e73 (byte[0x1220]=1 ∉ [8,15] → not skipped) |
| 6 | 0xff | (none) |
| 7 | 0xff | (none) |

## Implication for AC1mod (Phase 3)
To reconstruct a stage's full asset set: load the main PA (stage byte), then for each of
the 8 sub-resource bytes load entry `byte+ADDEND[slot]` (skip 0xFF). These are the
stage's chosen objects/MTs/effects — exactly the entries the mission then instances via
the block table (sub-trace D). Item B (mission → these 9 bytes) is the remaining piece.
