# AC1 player control — who is "the player", and how to dismount

Target **SLUS-01323 (v1.1)**. Static RE, 2026-10-19: offline `objdump` of the on-disc
FDAT 202 overlay (base `0x8004ADA0`) and the carved exe. `CONFIRMED` = read in disasm.
`HYPOTHESIS` = inferred. Model for on-foot play: the player entity is reskinned to a
human and shrunk, and keeps lock-box targeting and reskinned weapons.

**Verdict:** **[NEEDS NEW CODE]**. Not blocked for control. Recommended route:
**swap in place at slot 0** (the human *is* slot 0, and the parked AC becomes a new entity
in a free slot). Repointing control to another slot isn't viable: "the player" is slot 0
**by address** at 40+ sites.

---

## 1. How control reaches an entity  CONFIRMED
**Input is a global, not bound to an entity.**
- `InitPAD(0x8004A268, 8, 0x8004A270, 8)` in `main` (`0x80011F70..84`).
- `pad_read 0x80016A84` returns `port1 | port2<<16`.
- The mission tick `0x8007959C` calls `0x8007922C` (active set) → **`0x80079524` (pad
  sample)** → `0x8007863C`.
  - `0x80040FA0` = raw buttons, `0x80040FA4` = previous raw.
  - **`0x80040FA8` = gameplay buttons.** This is raw, except it's forced to 0 when
    `0x8019F524 & 0x400` (scripted sequence) is set or player AP `*(u16*)0x80041212 == 0`.
  - `0x80040FAC` = previous gameplay buttons.

**Behaviour comes from the vtable that player init installs on slot 0.**
- Scene load `0x8004FA08..1C`: `binder 0x80078B14(slot0, spawn template 0 @0x8019FAB8)`,
  then **`player_init 0x8007EAE8(slot0)`**.
- `player_init` copies AP to `+0x160/+0x162` from `0x80041212` and sets faction
  `+0x36 = 1`, flags `+0x38 = *0x80041226 & ~3`, `+0x164`, and so on.
- It then picks one of **two handler sets** by the byte at `0x8004121A`. HYPOTHESIS: leg
  class; `0` and `3` select A, `1` and `2` select B.

| slot | set A (`0x80080D20`) | set B (`0x80082BC4`) | role (per `COMBAT_PHYSICS.md` §6) |
|---|---|---|---|
| `+0x4C` | `0x80079888` (set elsewhere) | same | per-frame update wrapper |
| `+0x50` | `0x8007EF94` | `0x800813D8` | pad control / think |
| `+0x54` | `0x8008080C` | `0x8008238C` | movement state machine |
| `+0x58` | `0x80080D00` → `0x8007DF98` | `0x800826FC` → `0x8007DF98` | weapon/fire think |
| `+0x5C` | `0x8007DB98` | `0x8007DB98` | targeting / other |

Correction: the player's handlers are **in overlay 202, not the base exe**
(`ENTITY_AI_FRAMEWORK.md` slot map). The handlers take the entity in `a0`. They read the
**global** pad `0x80040FA8` and the player-singleton blocks:
- `0x80040FA8..0x80040FEC`: control state.
- `0x800411B0..0x8004127A`: AC stats, energy, ammo, the parts list at `0x80041278`.
- `0x80041020/24/34`: lock list.

**The vtable-installer pattern is the lever for on-foot play.** A third installer
(`set H`: on-foot `+0x50/+0x54/+0x58`) can be swapped onto slot 0 at dismount and back at
remount. That's the same mechanism the game uses for leg classes.

## 2. What's gated on being "the player" (slot 0 by address)  CONFIRMED (static scan)
Absolute slot-0 field references in overlay 202 (`lui 0x801A` + offset):

| subsystem | sites | fields |
|---|---|---|
| camera / HUD / radar (low overlay) | `0x800500AC..0x800517D8` (≈20) | `+0x08/+0x0A/+0x0C/+0x12` pos/yaw |
| HUD AP gauge | `0x800506E8/F0` | `+0x160/+0x162` |
| objective/location markers | `0x8008D48C..0x8008D5F4` | pos, yaw, `+0x2A` |
| aim/scope helpers | `0x80083048..0x800831EC` | pos, `+0x60` model, `+0x6C` |
| active-set (PVS) centre | `0x80079250` | `+0x16` → streaming, spawns, despawns, radar visibility |
| array base compares (`== slot0`) | 32 sites incl. `acquire 0x80073728`, router `0x800835FC`, lock scan `0x80073F58` | identity |
| damage | `0x80075350`: `target == slot0` ⇒ per-part player defense `0x80075280` (reads `0x80041214/15`) | identity |
| fire owner | fire handler passes constant `0x801A26B8` (`COMBAT_PHYSICS.md` §4) | identity |
| mission end | AP `0x80041212 == 0` kills input; result flow | global |

⇒ Moving control to slot *k* would mean patching all of the above. **Not worth it.**

## 3. Dismount plan (in place)  [NEEDS NEW CODE]
1. **Park the AC:** allocate a free slot (`0x80078A2C`) and copy slot 0's struct. Set
   faction, an inert or sentry think (`+0x58`), and HP from `0x80041212`. Model `+0x60`,
   pos `+0x08..0C`, extents `+0x28/+0x2A/+0x6A/+0x6C`.
2. **Turn slot 0 into the human:**
   - Install `set H` (or set A with on-foot stats).
   - Swap the model/geometry pointers (`+0x00/+0x04/+0x60`). OPEN: how the player model is
     assembled from garage parts.
   - Shrink extents (§4).
   - Swap the stat block `0x800411xx` (speed, energy, weapons). Save the AC's copy for
     remount.
3. **Remount:** the reverse, then free the parked slot (`despawn` path `0x80078D78`).
4. Things that follow slot 0 automatically: input, camera, HUD, radar, lock box, active set,
   and enemy targeting of the "player". That's why in-place wins.

## 4. Collision-scale sanity check (not solved)  CONFIRMED / OPEN
- **World collision (walls/floors) uses `+0x28` (half-width) and `+0x2A` (height).**
  `0x80074800(ent, pos, delta)` builds the AABB `{x±w, y−h, z±w}` from them and sweeps it
  with `0x80070F30`.
- **Hit-box (projectiles) uses `+0x6A/+0x6C`.** The binder seeds both pairs from the type
  data (`0x80078B98..BAC`). The player movement handler **re-copies `+0x28/+0x2A` into
  `+0x6A/+0x6C` every frame** (`0x80080758..0x80080770`), so writing `+0x6A/+0x6C`
  alone is overwritten.
- ⇒ **The lever is `+0x28/+0x2A`**, and the hit-box follows.
- `+0x2A/2` is also the eye/centre height for `los_ray 0x8007816C`, so a short human is
  harder to see over cover for free.
- OPEN: hardcoded mech-scale constants in the movement path (step-up height, ground snap,
  camera boom distance). Not surveyed. Start at `0x8008080C`, `0x80074800` and the camera
  sites in §2.

## 5. Open / live recipes (DuckStation)
- **C-1 (most informative next step) — the in-place shrink test.** Load any mission. After `player_init`
  (exec breakpoint `0x8004FA24`), write slot 0 `+0x28` (`0x801A26E0`) and `+0x2A`
  (`0x801A26E2`) to ¼ of their values and play.
  - Watch for: can the AC pass under or through geometry it couldn't before? Does anything
    restore the values (write-watch `0x801A26E0`)? Do `+0x6A/+0x6C` (`0x801A2722/24`)
    follow? Does the camera clip?
  - This answers the collision question, flushes out any hardcoded scale in one session,
    and needs no code.
- **C-2 — vtable swap smoke test.** Write set B's pointers onto a set-A player at runtime.
  Confirm the handlers are hot-swappable mid-mission (no re-init needed). That proves the
  `set H` mechanism.
- **C-3 — model assembly.** Exec breakpoint at the `+0x60` writers for slot 0 during scene
  load to find how the garage AC model is built. This is a prerequisite for the human
  reskin. It's the only part of Goal B that is effectively [BLOCKED] on RE.
- Identify `0x8004121A` (leg class?) by toggling it before `player_init` in tank/biped saves.
