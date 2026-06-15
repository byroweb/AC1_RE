# AC1 entity types, behaviour binding & objective conditions

Target **SLUS-01323 (v1.1)**. Static RE (offline disasm). Covers how a spawned
entity gets its behaviour, the per-mission objective object that supplies it, and the
win/fail condition vocabulary. Companions: `ENTITY_AI_FRAMEWORK.md` (entity struct),
`MISSION_SYSTEM.md` (mission runtime), `MISSION_SCRIPT_VM.md` (chunk-4 scripting),
`OBJECT_STATS.md` (spawn record fields). Tool: `tools/mission/type_catalog.py`
(→ `disc_map/type_catalog.csv`, `type_catalog_long.csv`).

## TWO type namespaces — do not conflate (CONFIRMED)

There are **two different "type" ids** per entity:

1. **Resource/class id = chunk-12 spawn record `hw7`** (template `+0x12`) — a global
   selector, range `0..388`, **264 distinct** values. Used as a bit key into the
   per-mission flag table `0x80198B08` (row = `hw9`×0x6c). **Not** copied to the live
   instance; **not** the behaviour dispatch.
2. **Logical dispatch type = chunk-12 `hw3`** (template `+0x0A` → entity `+0x0E`) — a
   *small* id (only `1..6` populated; `-1`/0 = none/stub). Indexes the objective
   object's per-type method columns. **This is the SAME field as the geometry block
   index** — the marshaller `FUN_80073B74` reshapes `hwK → template +0x04+2K`, and the
   binder reads `+0x0A` both as geom-record index and as the `+0x0E` dispatch type
   (`spawn_marshal`). HP = `hw11` → instance `+0x160/162/164`.

`type_catalog.py` keeps the namespaces separate; the meaningful join (mission, logical
type) is on **`hw3`**, not `hw7` (tool join is being updated). Anomaly: missions
**20/24/28** have an INIT table but **no chunk-12** (script-spawned/transition).

## Behaviour binding: how `+0x58` think gets set (CONFIRMED)

The per-entity vtable (`+0x4C..+0x60`, incl. `+0x58` think / `+0x5C` on-hit) is **not**
written by the generic spawn code — it is installed by a **per-mission INIT method**.

Spawn chain (all in mission overlay):
```
FUN_80078CFC(template)                       ; spawn entry (called by secondary VM / driver)
  -> FUN_80078A2C  allocator                 ; entity array 0x801A26B8, stride 0x170,
                                             ;   slots 1..15 (slot0=player); free = head word==0
  -> FUN_80078B14  binder                    ; copy template->instance, derive geom record
                                             ;   (template +0x0A -> 0x8019F538+idx*44),
                                             ;   then ZERO +0x4C/+0x50/+0x54/+0x58/+0x5C
  -> type dispatch @0x80078D50:
       type = instance +0x0E
       fn   = *( *0x8019F51C + 0x20 + type*4 )   ; per-type INIT column
       fn(instance)                              ; INIT installs +0x50..+0x5C incl. +0x58
```
The killed hypothesis: **`0x8019F590` is NOT a type table** (zero refs in the overlay).
A real type-keyed *defaults* table exists — **`0x80198808`, stride `0x18`** (per-type
size/bbox defaults, filled at load), used by registrar `FUN_800731F8`; it is data, not
a behaviour vtable.

## The objective object (FDAT entry 2N) (CONFIRMED)

Per-mission code+data blob, **always loaded to `0x801C4B40`** (no relocation; file
pointers are absolute `0x801Cxxxx`, `file_off = ptr − 0x801C4B40`). Pointed to by
`*0x8019F51C`. Its head is a pointer block:

```
+0x00 TICK(self)         per-frame objective method (driver FUN_8008AB68 @0x8008AB80)
+0x08 EVENT(id)          primary-VM cmd-9 dispatch
+0x0C QUERY(buf)         objective/HUD query
+0x10 FREE(entity)       entity destroy/despawn hook
+0x20 INIT[type]   (fn[], stride 4)   per-type spawn ctor; installs entity +0x58 think
+0x60 METHOD_B[k]  (fn[], stride 4)   index = entity +0x31 (sub-kind)
+0xA0 METHOD_C[t]  (fn[], stride 4)   index = placement-record +0x06
+0x120 AUX_TBL[8]  (fn[])             per-mission event-flag bit operators (HYP)
```
INIT column spans `+0x20..+0x60` = up to 16 logical types/mission (only 1..6 used).

### Per-mission fixed methods + type→think (CONFIRMED, missions 0–3)

| field | M0 | M1 | M2 | M3 |
|---|---|---|---|---|
| TICK `+0x00` | 801C5594 | 801C6994 | 801C6E54 | 801C6150 |
| EVENT `+0x08` | 801C558C | 801C6BE4 | 801C6DBC | 801C6090 |
| FREE `+0x10` | 801C558C | 801C6C20 | 801C6DBC | 801C6170 |

| mission | logical type | INIT fn | installed `+0x58` THINK |
|---|---|---|---|
| M0 | 1 | 801C52C0 | **801D8CBC** (via installer 801CAB4C; +0x54=801C75BC enemy AI) |
| M1 | 1,2 | 801C5E60 | **801C56A8** (installer 801C5D8C; +0x36=2) |
| M1 | 3 | 801C5E84 | 801C56A8 (+0x36=1) |
| M2 | 1 | 801C59E8 | **801C52A0** |
| M2 | 2 | 801C5E30 | **801C5B70** |
| M2 | 3 | 801C6528 | **801C5EA8** |
| M3 | 1 | 801C5374 | **801C4FB0** |

All think pointers land **inside** the objective-object region → confirms the
`type→think` map is per-mission data, not a global table. (`type_catalog.py` extracts
the INIT-pointer column for all 59 missions into `disc_map/type_catalog_long.csv`.)

## Objective TICK condition decode (CONFIRMED, M0–M3)

- **M0, M3 = intro-only**: tick is `jal 0x8008A80C` (the intro sequencer that counts
  phase `*0x8009079C` 0→37, firing sound/text cues). No win/fail in the tick — result
  comes from the **secondary VM `set_result`** opcode and/or external level-exit logic.
- **M1 = phase/escort state machine**: gated on `*0x8019F528==0`; uses RECORD-PRESENT
  / RECORD-TYPE on placement recs 36/52, ENTITY state (`+0x40`) and HP (`+0x160<4001`),
  heals/revives the escortee, commits phases via `FUN_8004C318(0x80)`.
- **M2 = timed multi-target-present gate**: when placement records 39–42 are all present
  (`rec+0x0A != -1`) and `type==3`, commit. (protect-all vs destroy-all sign = confirm live.)

### Condition-primitive taxonomy (the reusable check vocabulary)

| primitive | expressed as | source |
|---|---|---|
| STATUS-GATE | `*0x8019F528 == 0` | mission status word |
| READY-GATE | `FUN_80052338()` = `*(*0x801FD844+0x10)==0` | subsystem idle |
| RECORD-PRESENT | placement `rec[+0x0A] != -1` | `0x8019F538 + n*44` |
| RECORD-TYPE | placement `rec[+0x00] == T` | placement table |
| ENTITY-ALIVE/STATE | entity `+0x40` (`0xFF`=dead) | entity array |
| ENTITY-HP-THRESHOLD | entity `+0x160 < K` | entity array |
| ENTITY-IDENT-BY-INIT | `*(*0x8019F51C+0x20+type*4) == INIT_k` | kind tag via INIT ptr |
| DESTRUCTIBLE-COUNT | `*0x801D0B50` | world-struct counter |
| TIMER | `*0x8019F52C` / per-objobj counters | mission timer |
| PER-PHASE STEP | objobj-local "commit once" words | per-mission data |
| COMMIT-PROGRESS | `FUN_8004C318(0x80)` | objective sub-step |
| COMMIT SUCCESS/FAIL | `FUN_8004C318(0x100/0x200)` → `0x8019F524` | secondary VM / exit |
| EVENT-SPAWN | EVENT(id) → `FUN_80078CFC(rec)` | objobj event method |
| SCORE/KILL-TALLY | FREE hook adjusts objobj counters by kind | objobj free method |

Three result sinks: the objective tick's `0x80` progress commits, the **secondary VM**
`set_result` opcode (scripted ends), and intro/external level-exit + destructible logic.
**No M0–M3 mission commits `0x100` success from its TICK** — consistent with the
"Objective Complete" finding that destroying the last objective does not itself set
`0x8019F524`.

## Corrections folded in (2026-06-15)
- `FUN_80052A2C` = TEXT/PROMPT VM, not a "condition 99" predicate (see `MISSION_SYSTEM.md`).
- `0x8008BAF8` = secondary-VM `set_result` opcode body (see `MISSION_SCRIPT_VM.md`).
- Real ready/idle gate = `FUN_80052338`.

## Open / confirm-live
- ~~chunk-12 field → template `+0x0A`~~ RESOLVED: it is `hw3` (`spawn_marshal`).
- Concrete numeric type→think for all 59 missions (extend `type_catalog.py` to follow
  INIT bodies, or breakpoint `0x80078D50` live).
- AUX_TBL `+0x120` per-mission bit→flag semantics; `0x80198808` row format.
- M2 record-39–42 protect-vs-destroy sign; `0x801FD844` subsystem identity.
- Missions 20/24/28 (no chunk-12): how their entities are introduced.
