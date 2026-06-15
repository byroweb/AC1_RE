# AC1 Entity / NPC / AI Framework — RE notes

Target **SLUS-01323 (v1.1)**. Live ground-truth in DuckStation inside the
**train-escort mission** (save slot **1** = mission start; **SS3** = the
"frozen diorama" checkpoint produced below). Static disasm of the in-mission
overlay (FDAT 202, `0x80050000`–`0x800DFFFF`) read live.

Notation: `CONFIRMED` = disasm/live-verified today. `HYPOTHESIS` = inferred.

Goal of this session was a **reusable toolkit for authoring missions**, not this
level's trivia: how moving objects, enemy AI, target selection and damage are
wired, and how to switch any of it off to inspect a scene.

---

## 0. The one big idea — everything is an "entity"

Every dynamic actor in a mission (player, enemy AC, aircraft, train) is one
record in a **single entity array**:

- base **`0x801A26B8`**, **stride `0x170` (368 bytes)**  CONFIRMED
- the damage router (§4) bounds-checks the AC range as
  `[0x801A26B8, 0x801A26B8 + 0x1590)` and `0x1590 = 15 × 0x170` exactly → up to
  **15 entity slots**.

The radar/target scan loop iterates a hard-coded count and the damage router
bounds-checks an exact `0x1590 = 15 × 0x170` window → the array is a **statically
sized, fixed-stride table of ~15 slots** (player + up to ~14 others), not a dynamic
list. See `RAMWATCH_CONCORDANCE.md` §D3 (15-vs-16 loop-bound check still open).

They share ONE struct; only the **vtable** (behaviour function pointers) and the
**stats** differ. Consequence: there is **no special "moving" or "destructible"
class**. Movement = a think-handler pointer; destructibility = an HP field + a
damage handler. The static `0x801D0B68` world props (generators/fuel tanks, see
`OBJECT_STATS.md`) are the *degenerate* entity — null mover, world-route damage;
the train is the *full* case — mover + HP, on the entity route.

> Stride `0x170` is independently corroborated in
> [RUNTIME_RAM_MAP.md](RUNTIME_RAM_MAP.md) (the project's method-table test + Zinfidel
> BizHawk `ENTITY_OFFSET = 0x170`). Today's mission data agrees: slot 7 = train at
> `+7*0x170`, HP/faction/vtable all align, and the damage router's AC range is an
> exact `0x170` multiple. The separate `0xD0`-stride / 510-slot pool at
> `0x801A25E8` (`COMBAT_PHYSICS.md` §1) overlaps at the player address but is
> indexed differently — how the two relate at runtime is still **open** (tracked
> in `RUNTIME_RAM_MAP.md`).

### This mission's slot map  CONFIRMED
| slot | addr | actor | think handler (`+0x58`) |
|---|---|---|---|
| 0 | `0x801A26B8` | player | base-exe control funcs |
| 1 | `0x801A2828` | aircraft (type A) | `0x801C66DC` |
| 2 | `0x801A2998` | **red enemy AC** | `0x801C5B7C` |
| 3–6 | `0x801A2B08…0x801A2F58` | aircraft (type B) | `0x801C68E4` |
| 7 | `0x801A30C8` | **train** | `0x801CBD44` |

### Entity struct fields  CONFIRMED
| off | size | meaning |
|---|---|---|
| `+0x00` | ptr | definition/resource pointer (`*(entity)`; armor stat at its `+0x14`). ⚠ TASVideos reads this as u16 "ID" `+0x00` + u16 "active flags" `+0x02` — the ptr's low half varies by type (looks ID-like); needs reconcile (see `RAMWATCH_CONCORDANCE.md` §D2) |
| `+0x04` | ptr | geometry-block record table (`0x8019F5xx`) |
| `+0x08`,`+0x0C` | s16 | render/screen-space position (rebuilt each frame from matrix; used by target scoring) |
| `+0x12` | s16 | **yaw / facing** (corroborated by TASVideos player-yaw `0x801A26CA`; re-verify) |
| `+0x20`,`+0x24` | 16.16 | **train's** authoritative world X/Z (offset varies by entity type) |
| `+0x36` | u16 | **faction/team id** in low 3 bits (red AC = 2). Friend/foe knob |
| `+0x40` | u8 | **AI state byte** (state-machine selector; `0xFF` = special branch) |
| `+0x48` | ptr | **current target entity** (recomputed each scan — §3) |
| `+0x4C…+0x60` | ptr×… | **vtable**: …/ **think `+0x58`** / **on-hit+death `+0x5C`** / sub `+0x60` |
| `+0x148` | u16 | fire cooldown timer |
| `+0x14A/B/C` | u8 | fire timing params |
| `+0x160` | u16 | **current HP / AP** |
| `+0x162` | u16 | **max HP / AP** |
| `+0x15C` | u32 | AI flag bitmask;  `+0x3E/+0x3F` = AI sub-state |

HP seen: red AC **1200** (`0x801A2AF8`), train **9200** (`0x801A3228`).

---

## 1. The generic entity updater  CONFIRMED

`~0x800789D0` runs per entity per frame:

```c
v0 = entity->think;          // +0x58
if (v0 != 0) v0(entity);     // 0x800789EC jalr — handler writes next pos to caller stack
entity->worldpos = stack_pos;// 0x80078A04..A10 swl/swr to +0x20/+0x24 (UNCONDITIONAL)
```

The position write is **unconditional** — the handler's job is to leave the new
position in the caller's stack frame. This matters for the freeze tool below.

Per-entity think handlers: player = base-exe; aircraft = `0x801C66DC` /
`0x801C68E4`; red AC = `0x801C5B7C`; train = `0x801CBD44` (a path-follower that
reads the mission timer `0x8019F52C` and advances at constant velocity = on
rails).

---

## 2. Freeze tool — "AI off" for any entity  CONFIRMED (reusable)

Nulling `+0x58` alone **corrupts** position: the updater still writes stale stack
into world-pos (observed drift / teleport). Fix with a one-instruction patch so a
null handler also skips the write:

```
0x800789E4:  10400003 -> 1040000B    # beq v0,zero target 0x800789F4 -> 0x80078A14
                                      # (skip the +0x20/+0x24 store block)
```

Then **write 0 to any entity's `+0x58`** to freeze it cleanly (no move / AI /
fire / corruption); restore the original pointer to un-freeze. Validated: train
world-X held exactly across frame-steps and at full speed. All 7 NPCs frozen →
player free to roam, energy stable.

Notes: `0x80078xxx` is in the swappable mission overlay, so the patch is
RAM-only / non-persistent (reload restores it). Damage/collision is **independent
of `+0x58`**, so frozen entities still take hits (§4) — ideal sitting targets.

---

## 3. Target selection / arbitration  CONFIRMED

The enemy's target (`entity+0x48`) is **not fixed** — it is recomputed by a
nearest-enemy scan at **`0x80073774`–`0x800738C0`** that writes the winner with
`sw s7, 0x48(redAC)`:

```c
best = none; bestScore = 0x1FFFFFFF;
for (cand in entities) {
    if (*cand == 0) continue;                      // invalid/dead
    if ((cand->+0x36 & 7) == self->faction) continue;   // same faction -> skip
    if (cand == player && (*0x8019F524 & 0x400)) continue; // player-exclusion flag
    if (cand == self) continue;
    score = (dx>>1)^2 + (dy>>1)^2 (+dz via 0x80027324);  // squared distance, +0x08/+0x0C
    if (score < bestScore) { bestScore = score; best = cand; }
}
redAC->+0x48 = best;
```

So an enemy locks onto the **closest non-allied entity**, freely switching
between player / train / aircraft. **Two designer levers:** the **`+0x36`
faction id** and the **`0x8019F524 & 0x400`** player-untargetable flag (`0x8019F524`
is the mission success/fail flag word — see `MISSION_SYSTEM.md`).

---

## 4. Damage path & calculation  CONFIRMED (live, train hit by MG round)

Projectile collision → region dispatch **`0x800835E8`** routes by target address:

```c
if (target in [0x801A26B8, 0x801A3C48))   // the entity array
     0x80075350(target, projectile, ...); // shared AC/entity damage  <-- player route
else // static 0x801D0B68 world props
     world-destructible handler @ target+0x2C;  // the "pre-calculated" route
```

The red AC / train / aircraft are all in the entity array, so they take damage
through the **same handler the player uses** (`0x80075350`), NOT the destructible
route. `0x80075350`:

```c
if (target == player_slot0)  dmg = 0x80075280(projectile);     // rich per-part AP routine
else                         dmg = 0x800752F4(*(target)->+0x14, projectile); // NPC: single armor stat
hp = target->+0x160; hp -= dmg; if (hp < 0) hp = 0; target->+0x160 = hp;
if (target->+0x5C) target->+0x5C(target, projectile, ...);     // on-hit / death reaction
```

So the only player/NPC difference is the **damage-amount sub-calc**: the player
uses the detailed per-part armor routine `0x80075280`; NPCs use a **single
defense stat from their definition** (`*(entity)->+0x14`) via `0x800752F4`.
Same function, same `+0x160` HP field, same `+0x5C` death hook for both.

---

## 5. Red AC AI — the "complex enemy" template  CONFIRMED

Think handler `0x801C5B7C` per frame:

```
state = entity->+0x40                         // 0xFF = special branch
sense/aim sub-AI 0x801C59F8:                   // reads target +0x48
    LOS/aim check 0x80073C2C(target)
    sensor/range  0x800745D0
RNG 0x8002A4A0 gates the decision (e.g. if rand < 2622 commit)
on commit: FIRE 0x80077B78(weapon_id a1=23, params from *(entity) def)
           arm cooldown +0x148 = 15, timing +0x14A/B/C = 180/160/140
bookkeeps sub-state +0x3E/+0x3F and flag bits +0x15C
```

**Reusable engine AI services** (the level-AI toolbox):
| addr | service |
|---|---|
| `0x80077B78` | fire weapon (id in `a1`) |
| `0x80073C2C` | aim / line-of-sight to target |
| `0x800745D0` | sensor / range check |
| `0x8002A4A0` | RNG |

Aircraft (`0x801C66DC` / `0x801C68E4`) are the same shape with a thinner
decision tree.

---

## How to reproduce
1. `load_state` slot 1; run at 0.25× until the cockpit/gantry scene loads and
   slot 7 (`0x801A3120` = `0x801CBD44`) populates (train spawned).
2. Apply the §2 patch + null the `+0x58` of slots 1–7 to freeze the scene
   (= **SS3**).
3. Watchpoint `+0x48` and reactivate slot 2 to catch target selection (§3).
4. Breakpoint `0x800835E8` / `0x80075350`, shoot a frozen NPC to catch the
   damage route (§4); HP at `+0x160`.

See also: `COMBAT_PHYSICS.md` (player AC fire chain / projectile pool),
`OBJECT_STATS.md` (static `0x801D0B68` destructibles), `MISSION_SYSTEM.md`
(timer / success flag `0x8019F524`).
