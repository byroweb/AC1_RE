# AC1 Destructible objects — generator/"energy infrastructure" (WORKER ROBOT REMOVAL)

Session 2026-06-13. Live RE in DuckStation (MCP) + headless Ghidra on the
carved mission overlay `disc_map/overlays/ovl202_mission.bin` (base **0x8004ADA0**,
via `tools/ghidra_overlay.sh`). Cross-ref: `docs/OBJECT_STATS.md`,
`docs/COMBAT_PHYSICS.md`, `docs/MISSION_SYSTEM.md`.

Goal: how the game encodes destructible terrain (the generators you must avoid
damaging in WORKER ROBOT REMOVAL), and what happens on destruction.

## TL;DR  (SOLVED + ground-truthed)
The generator is a **destructible world-structure object** — a record in the array
at **`0x801D0B68`** (stride **0x40**, count `_DAT_801E8B6C`). The WORKER-ROBOT-REMOVAL
generator I tested = **record 162 @ `0x801D33E8`**. It is NOT an MT-spawn object
(`0x8019FAB8`) and NOT an active-object/AC (`0x801A26B8`) — which is why those
diffs were empty. It **does have HP**, but the HP lives in a **stats sub-struct**
pointed to by record `+0x38` (here `0x801DA620`), not in the record itself — which
is why no region diff caught it (none covered `0x801DA620`).

Verified live: generator **HP = 10** (intact) → **`0xFFFF`** (destroyed sentinel).
Pulse-rifle round damage ≥ 10 ⇒ one-shot (matches what we saw on screen).

### The full hit→destroy chain (all CONFIRMED via decompile + live)
1. Round think **`FUN_80083A10`** (installed at projectile `+0x14` by spawn
   `FUN_80085E98`) integrates the round, then calls collision query
   **`FUN_8006D534`** (→ `FUN_80073598` / `FUN_80074728`); on a hit it returns the
   struck target pointer and sets projectile `+6 = 1` (despawn).
2. On hit, `FUN_80083A10` calls **`FUN_800873F8`** (spawn impact **explosion** at the
   hit point — the fireball) and **`FUN_800835E8`** (apply the hit to the target).
3. **`FUN_800835E8` dispatches by the target's RAM region** (this is how object
   classes are told apart):
   - `0x801A26B8`–`0x801A3C48` → active object/AC → `FUN_80075350` (AC damage)
   - `0x80198B08`–`0x8019F2D4` → handler at target `+0x14`, call `*(handler+4)`
   - **`0x801D0B68`+ (world structures) → handler at target `+0x2C`, call
     `*(handler+4)` with message `a1 = 0x8000` (damage)**
4. Generator's handler fn = **`0x801C58C4`**, located in the **per-mission
   objective-object overlay** (`0x801C0000+`, FDAT entry `2N`, vtable `~0x801C4B40`) —
   NOT the shared mission overlay. It does:
   `hp(@sub+0) -= damage(@dmginfo+2);  if (hp>0) survive;  else destroy:`
   set `hp=0xFFFF`, compute bbox center `(min+max)/2` from record `+0x18/+0x20…`,
   spawn explosion via **`FUN_800869F0`** + sound/shrapnel (params at sub `+2..+5`).
5. The structure record is then retired: geometry/bbox fields → **`0x7FFF`** sentinels
   and the handler pointer at **`+0x2C` → 0** (removed from render + collision).

### Destructible world-structure record layout (0x40 B, `0x801D0B68 + i*0x40`)
- `+0x18/+0x1A/+0x1C` = bbox min XYZ; `+0x20/+0x22/+0x24` = bbox max XYZ
  (used for collision and as the explosion center).
- `+0x2C` = on-hit handler pointer (→ small struct; **fn at `+4`**). **Nulled on
  destroy** = the "dead" marker.
- `+0x38` = pointer to **stats sub-struct**: `+0` u16 **HP**, `+2` effect-enable,
  `+3` explosion model id, `+4/+5` sound flag/id.

> NOTE: this supersedes the earlier "no HP / MT object" notes further down — those
> were the wrong-path hypotheses recorded mid-investigation; keep them only as a
> record of what was ruled out. The MT spawner fact (`+0x02 = +0x1A`) remains a
> correct detail about the *MT* record layout, just unrelated to the generator.

## Record layout (re-derived this session — refines OBJECT_STATS.md §1)
`FUN_80073B74` (`0x80073B74`) initialises each 44-byte record:
- `+0x00` (byte) = **0** (status/flags; runtime-set).
- copies the 20 chunk-12 source halfwords into `+0x04 .. +0x2B`.
- `+0x02` (hw) = `+0x1A` (hw)  ← i.e. **live field at +0x02 seeded from template hw11**.

> DISCREPANCY to reconcile: OBJECT_STATS.md §1 records the derived copy as
> `+0x00 = +0x18` (hw10). The current headless-Ghidra decompile of `FUN_80073B74`
> clearly shows `*(rec+0) = 0` and `*(u16*)(rec+2) = *(u16*)(rec+0x1A)`. Since
> `hw11`/`+0x1A` is the standing **HP candidate**, `+0x02` is the prime "current HP"
> field. NEEDS live ground-truth (watchpoint, below) before promoting to CONFIRMED.

## Destruction behaviour — MT event/script processor `FUN_8008B8xx`
The per-frame processor that walks `0x8019FAB8` (refs at `0x8008B834`, `0x8008B900`)
runs each object's action bitmask. Bits that match the observed explosion:
- **0x8000** — loops a coordinate list, spawning sub-objects via `FUN_80078250` /
  `FUN_8007832C` with randomized `(rng&0x7f)+300`, color `0x42`. → the **2D
  shrapnel burst** (rotating sprites that fall + despawn).
- **0x4000** — periodic spawn via `FUN_8004C98C` (period = `hw[0x12]`). → effect
  emitter (**3D expanding explosion** / smoke).
- **0x80000** — `_DAT_8019F524 |= 0x400` then `FUN_800796F4`. → sets a
  **mission-event flag** in the success/fail word `0x8019F524` (relevant to the
  no-collateral-damage objective scoring).
- 0x100/0x200/0x400/0x800/0x10000/0x20000/0x40000 — other scripted actions
  (sound, sub-object spawn, links).

## Render side (where the effect is VISIBLE in RAM)
The explosion + shrapnel draw from **double-buffered screen-space sprite lists**
alternating between **~0x8014C000** and **~0x8016A000** each frame, as ~40-byte
(0x28) projected quad records. Frame-stepping shows the coords expand (fireball
growing) then shrink/fall (shrapnel) before despawn. These are GTE-projected
render output — NOT entity state — and they (plus per-frame transform buffers at
0x80157/0x80172/0x80188/0x80190xxx) **flood any running-window memory diff**, which
is why live snapshot/diff cannot isolate the entity cleanly. Use Ghidra / a
watchpoint instead.

## Function reference (overlay ovl202 unless noted)
- `FUN_80085E98` — projectile spawn; installs think `FUN_80083A10` at proj `+0x14`.
- `FUN_80083A10` — round think: integrate + collide + explode-on-hit.
- `FUN_8006D534` — collision query (`FUN_80073598` world?, `FUN_80074728`); returns hit target.
- `FUN_800873F8` — spawn impact explosion at hit point.
- `FUN_800835E8` — apply hit; region-dispatch (AC pool / `0x80198B08` class / `0x801D0B68` world class).
- `FUN_80075350` — AC/active-object damage.
- `0x801C58C4` — **generator on-hit/destroy handler** (per-mission objective overlay
  `0x801C0000+`, FDAT 2N). HP-decrement + destroy + explosion `FUN_800869F0`.

## Follow-ups (optional)
- Decompile `FUN_80073598`/`FUN_80074728` to document how world-structure bbox
  collision is tested (they're what returns the `0x801D0B68` target).
- Confirm pulse-rifle round damage value (`*(dmginfo+2)`) vs the 10 HP, and whether
  other generators in the level carry different HP (read each `0x801D0B68` record's
  `+0x38`→`+0` across the array; count = `_DAT_801E8B6C`).
- The `0x80198B08`–`0x8019F2D4` class (handler at `+0x14`) is a *second* destructible
  family — identify what objects live there.

## Tooling / emulator notes (for repeatability)
- Pad is sampled **only while running**; `press_button`+`frame_step` does NOT fire.
  Fire via `input_sequence` while running. The **round flies autonomously** after
  the trigger, so: drop speed (`set_speed 0.01–0.05`), fire, pause, then
  `frame_step` the flight/explosion frame-by-frame.
- Round reaches this generator in **~1–2 frames** (near-instant at range).
- `input_sequence` state **persists across save-state loads** (reports "already
  active") — reset/avoid stacking.
- Save states: **slot 1** = intact (level start); **slot 2** = explosion-in-progress
  (saved this session).
- The live Ghidra DB has the **front-end overlay (201)** banked into
  `0x80050000–0x800DFFFF`, NOT the mission overlay — combat/mission functions are
  absent there. Use `tools/ghidra_overlay.sh disc_map/overlays/ovl202_mission.bin <addr>`.
