# AC1 Combat / Physics / Missiles — RE notes

Target **SLUS-01323 (v1.1)**. Mix of static (Ghidra DB = FDAT entry-201 / base
exe) and **live ground-truth in DuckStation** inside the first Raven-test arena
(save state **slot 10** = arena, paused; backup of all slots in
`savestate_backup_20260609/`).

Notation: `CONFIRMED` = byte/disasm/live-verified. `HYPOTHESIS` = inferred.

---

## 0. How to get here (live)
- Boot disc, `load_state` **slot 1** (Raven-test intro text) → `Start`/`Cross`
  through the briefing → "Now Loading" → arena. Pause immediately.
- `slot 10` is the arena already loaded + paused (full AP). Reload it whenever the
  fight goes bad — combat is fast and the enemy AI will wreck you (work paused +
  `frame_step`; AP/timer only tick while running).
- HUD: green vertical bar + number = **AP** (defense, decreases when hit). Left
  also shows **ENERGY** (boost gauge, drains/regens). Top = mission timer
  (lose if AP **or** timer hits 0). Right scope = current weapon + ammo
  (RIFLE 200/200, SMALL MISSILE 40/40). Controls: **Square** = fire selected arm
  weapon (rifle = full-auto, hold); **Triangle** = cycle weapon; missile only
  fires **when locked** (can't blind-fire).

---

## 1. The shared game-object pool  CONFIRMED
A fixed array of **510 slots × 0xD0 (208) bytes**:
- base `0x801A25E8`, end bound `0x801BC448` (`slot 510` addr; valid slots 0..509).
- Used by whichever active overlay's frame loop is running (menu/garage AND
  in-mission both index this same base).

**Player AC = slot 1 = `0x801A26B8`** (CONFIRMED live: appears as `s3` in the
player transform code while moving; position fields track movement). Its first
two words are shared-table pointers:
`+0x00 = 0x8019FAB8` (MT/instance template table), `+0x04 = 0x8019F538`
(geometry-block record table) — see `docs/MISSION_SYSTEM.md` / `PA_HEADER.md`.

### Player AC object layout (live-confirmed fields)
| off | meaning | status |
| --- | --- | --- |
| `+0x00` | ptr → template table `0x8019FAB8` | CONFIRMED |
| `+0x04` | ptr → geometry record table `0x8019F538` | CONFIRMED |
| `+0x08` | **world position X** (int32; constant while moving straight fwd) | CONFIRMED |
| `+0x0C` | **world position Z** (int32; integrates forward, see §3) | CONFIRMED |
| `+0x14` | **world position Y / height** (`0x210000` at rest) | HYPOTHESIS |
| `+0x20`,`+0x24` | second X/Z copy (prev-frame / smoothed) | HYPOTHESIS |
| `+0x94` | velocity-ish (large per-frame delta when accelerating) | HYPOTHESIS |

The Z position accelerated `+68` then `+137` per equal 15-frame Up burst → genuine
**velocity ramp / acceleration**, i.e. a real integrator, not teleport.

### Render/think model (static, from the menu frame loop `0x8009BD64`)
The menu/garage loop (renamed `main_frame_loop`) iterates the pool at stride
0xD0 and, per active object, calls method pointers and dispatches a render type:
- `obj+0x74` = **per-frame think/update fn ptr**; `obj+0x78` = despawn fn ptr.
- `obj+0x0A` = render-type selector (0=model `FUN_80057190`, 1,3,4,5,7,8 = other
  model/particle renderers, 2 = `farsi_gte_renderer_type6`, 6 = `draw_string`).
- `obj+0x20` = MATRIX, `obj+0x40` = SVECTOR rot, `obj+0x48` = pos VECTOR,
  `obj+0x58` = scale, `obj+0x6c` = parent ptr, `obj+0x68` = attach link.
- helper `mat_set_translation` (`0x80028074`) copies a VECTOR into MATRIX.t (+0x14).
> NB: this loop's code is **not resident during a mission** (RAM at 0x8009BD64
> reads as zero in-arena) — it is the menu overlay. The in-mission frame loop is
> in the mission overlay and reuses the same pool; its exact per-object dispatch
> is not yet pinned, but the (+0x74 think / +0x0A render-type) template is shared.

---

## 2. What runs each frame (live PCs, mission overlay)
Captured by write-watchpoints while walking the player forward (frame-stepped):

- **Player → view/render globals export** `~0x8007BB80`..`0x8007BC68` CONFIRMED.
  Copies the AC's pose from a stack-local state struct (`s0`) into the global
  block `0x80041020..0x80041048` (`sh` to `0x80041034/36/38/3c/3e/40`, then an
  `lwl/lwr/swl/swr` block copy of `0x80041034..0x8004104B`). `s3 = 0x801A26B8`
  (player). This is the camera/HUD/world-matrix source block, **not** the
  authoritative AC position (that's the object at `0x801A26B8`).
  The global `0x80041030/34/38` triple = the exported view position.

- **Hot per-frame transform/collision routine** `0x800674B0` CONFIRMED present.
  Prologue `addiu sp,-64`; args `a0=struct@0x801D0DE8`, `a1=0x24`; uses
  **scratchpad** (`0x1F800018/50`), reads object table @ `0x801AEB30`. Called from
  `0x8005CB24`. Touches the player object during movement (trapped by the
  `0x801A26C4` write-watch). HYPOTHESIS: this is the movement-integration /
  collision step (matrix build + position commit). Exact store TBD (watch landed
  on the prologue — likely an unaligned block store inside the body).

Resident idle/vsync wait loop sits at `0x8002E198` (where the CPU parks when
paused between frames).

---

## 3. Physics integrator  (status: anchored, not fully decoded)
Confirmed behaviour: holding forward makes `objZ (+0x0C)` increase with a rising
per-frame delta (acceleration), X (`+0x08`) and Y (`+0x14`) steady → standard
`pos += vel; vel += accel` integration on the AC object, executed inside the
mission-overlay per-frame code reached via `0x8005CB24 → 0x800674B0`. **Next:**
re-trap the `0x801A26C4` write with a hardware-style watch and single-step the
body to capture the `pos += vel` and the boost/gravity terms (energy gauge feeds
boost). Tool path: slot 10 → write-watch `0x801A26C4` → step into body.

---

## 4. Weapon-fire chain  CONFIRMED (live trapped)
Found by write-watching the **rifle ammo counter `0x8004128A`** (= ammo struct
`0x8004127A` + 0x10) and firing Square. Full chain:

1. **Fire handler** `~0x8007CA0C..0x8007CD40` (per-hardpoint loop). Per shot:
   `ammo = *(0x8004128A); ammo--; store` (CONFIRMED at `0x8007CCD8..E4`). On the
   shot it calls the spawn fn (below) then an FX/recoil fn `FUN_80089F7C`
   (a0 = view-global `*0x80041024`, a2 = player obj `0x801A26B8`), and writes the
   **muzzle-flash colour (180,160,140)** to player obj `+0x14E/+0x14F/+0x150`, plus
   a per-weapon cooldown `sh 0,0x74(player)` (`0x801A272C`).
2. **Projectile spawn** `FUN_80085E98(a0,a1,a2=player,a3, [stack: …, posPtr=0x801A26C0,
   playerObj])`:
   - calls **`FUN_80089ECC` = projectile-pool ALLOCATOR**; returns the round obj,
     or sentinel `0x8008EC88` ("pool full") → bail.
   - calls **`FUN_80085764` = projectile INITIALISER** `(round, ammoStruct,
     weaponDef=0x8008E530, -1)` — stamps type/speed/think from the weapon def.
   - sets round `+0x10 = 1` (active), copies weapon-def words into round `+0xB8`.

### Projectile pool  CONFIRMED
`FUN_80089ECC`: base **`0x801AEB3C`**, **256 entries × `0xE0` (224 bytes)**, a slot
is **free when `int16 @ +0x06 == -1`** (linear scan for the first free). This is a
*separate* pool from the AC/object array — that's why firing produced no new slot
in the `0x801A25E8` array. The init branch selects the round's **per-type handler
/ think function from tables near `0x8009938C` / `0x80099B44`** (and `0x80085454` /
`0x80085510`) indexed by projectile type → **the missile's homing update is one of
these type-table entries.**

### Weapon definition table  CONFIRMED (location) / HYPOTHESIS (fields)
`0x8008E530`, entry stride ~`0x1C` (28 B). Rifle entry bytes:
`D1 00 | 40 01 | 10 78 | 14 00 | BC 02 00 00 | 1E 00 | 02 00 | 04 00 | C0 00 |
05 04 01 00 | FA 64 FA 00`. Plausible: `+0x00` type/model `0x00D1`, `+0x02` speed
`0x0140`, `+0x08` range/life `0x02BC`=700, `+0x0C` damage `0x1E`=30, `+0x18` tracer
RGB `(250,100,250)`. Next entry (`4A 00 40 01 10 50 10 00 BC 02 7F 00 …`) = another
weapon. The **missile def is another entry here**; its type selects the homing think.

## 5. Projectile per-frame think + MISSILE HOMING  CONFIRMED (live)
Each round in the pool carries a **think fn ptr at `+0x14`** (the projectile
struct's "update"); position `+0x00/+0x02/+0x04`, velocity `+0x1C/+0x1E/+0x20`
(int16). Three kinds observed in flight (snapshot diff of the pool over frames):

| think (`+0x14`) | kind | behaviour |
| --- | --- | --- |
| **`0x800851A8`** | dumb/ballistic round (rifle) | pure `pos += vel`: adds `+0x1C..20` to `+0x00..04` (and a fine sub-pos `+0xB8 += +0x24`) every frame. Velocity **constant** → straight line. CONFIRMED (full disasm). |
| **`0x80085510`** | other round type | (act-flag 3; not decoded) |
| **`0x8008523C`** | **GUIDED MISSILE** | advances a turn phase/angle (`+0x1A` += step `+0x19`, `+0xBA`, `+0xB8`), indexes per-type turn tables (`0x80039C7C[type<<3]`, `0x8003FB48`, `0x80039640`), `mult(+0xBA, table[+0x1A]) >> 10`, and calls trig/rotate helper **`0x80056B54`** with the missile + heading `+0x0E`. CONFIRMED. |

**Homing toward the locked target** — write-watching the missile's velocity
(`pool slot+0x1C`) while it flew trapped the steering routine **`0x8005C904`**:
takes the missile work-struct (`a0`) **and the target AC object** (`a2 ≈ 0x801AC860`,
an entry in the AC array = the locked enemy) and uses matrix/GTE helpers
`0x800281D4` / `0x80028264` to **rotate the missile heading toward the target**,
writing the new velocity. So:

> **small-missile flight = guided-projectile think `0x8008523C` → steer-toward-
> target `0x8005C904` (reads the locked enemy AC, builds a rotation, turns the
> velocity vector that much per frame) → the shared `pos += vel` advance.** It is
> true target-tracking homing (not a fixed curve); turn-rate is capped by the
> per-type turn table. Lock is mandatory because the steer routine needs the
> target-AC pointer.

Projectile struct (0xE0): `+0x10`=class/active (1=ballistic round, 2=missile,
3=other), `+0x14`=think fn, `+0x18/19/1a/1b`=type / step / phase counters,
`+0x06`=free marker (`-1`=free), `+0xB8..`=fine position / def copy.

## 8. Damage model  CONFIRMED (headless Ghidra + live-verified)

### Why AP hid from every RAM search
The HUD "AP" (e.g. 4408) is **stored at 4×** the displayed value, as a `u16` at
**`playerAC + 0x160`** (`0x801A2818`). Live-verified: HUD 4408 ⇔ `0x801A2818 = 17632
= 4408×4`. Searching for 4408/4184/3624 (or any sum) never matched because the real
value is 17632/16736/14496. AP is a **single total pool**, *not* per-part.

### The hit → damage chain
1. **Projectile think `FUN_80083a10`** (installed at round`+0x14` by spawn
   `FUN_80085E98` — note the per-type integrators like `0x800851A8` are *sub-steps*,
   not the top-level think). Each frame it raycasts the round's path:
   `hit = FUN_8006d534(world, round, Δ, nextPos)`.
2. On a hit, **`FUN_800835e8(owner, hitObj, …)`** dispatches by object type:
   AC-array range `[0x801A26B8, 0x801A3C48]` → `FUN_80075350`; other ranges →
   the object's own vtable hit method.
3. **`FUN_80075350(ac, atk, …)`** — the AC damage apply:
   ```c
   dmg = (ac == player) ? FUN_80075280(atk) << 1     // player defense calc, then ×2
                        : FUN_800752f4(ac->tmpl[0x14], atk);  // enemy defense calc
   u16 *AP = (u16*)(ac + 0x160);
   *AP = (*AP < dmg) ? 0 : *AP - dmg;                 // subtract, clamp at 0
   if (ac->hook[0x5c]) ac->hook[0x5c](ac, atk, …, dmg); // part-destroy / death FX
   ```

### Damage type & the part defenses
The projectile carries a **damage descriptor**: `u16 attack` at `desc+2`, and the
**damage type** in `desc[0] & 0x30` (`0x10` vs `0x20` = the two categories,
shell / energy — which-is-which TBD via a `weaponDef 0x8008E530` check).

- **Player** `0x80075280`: `def = (type==0x10 ? *0x80041214 : type==0x20 ?
  *0x80041215 : none)`; if a type matched, `eff = (attack × def) >> 6`; then
  `return (eff×2)/3`; the caller then `<<1`. The two bytes `0x80041214/15` are the
  AC's **aggregate shell-def / energy-def**. Live-disasm CONFIRMED at these exact
  addresses (`lbu v0,0x1214(0x8004)` / `0x1215`). Lower byte ⇒ less damage taken
  (`mult = def/64`). Live: `27` (type 0x10), `32` (type 0x20).
  **These two bytes are derived per-part defenses summed + scaled — see §8a.**
- **Enemy** `0x800752f4`: a signed defense nibble in `ac->tmpl[0x14]` bits 8–11
  (`def∈[-8..7]`); for type 0x20, `eff = attack × (8 − def)/8 + 1` (higher def ⇒ less).
  Live-disasm CONFIRMED: `lhu a0,0x14(tmpl)`, `andi a0,0xF00`, `srl 8`, sign-extend.

**So the answer to "how do part energy-def/shell-def factor in":** the AC's two
defense bytes are **derived from the equipped parts** at mission load — an integer
accumulation of per-part fields run through a softfloat curve + clamp (see §8a for
the producer). ⚠️ Whether that nets out to "more parts = tankier" depends on the
curve's *sign*, which is **NOT yet decoded** — note the runtime makes `dmg ∝ def_byte`
(lower byte = less damage), so the curve must invert the sum; see §8a's UNRESOLVED
box. **Only the player takes this part-derived path — every enemy AC uses a single
template nibble `tmpl[0x14]` instead** (see §8b). On a hit the engine reduces the raw
weapon `attack` by the defense **matching the weapon's damage type**, then subtracts
the result from the single AP pool at `AC+0x160`. The `+0x5c` post-hit hook is where individual part
loss / destruction visuals and the death sequence are driven.

> **Live-disasm note (2026-06-10):** decode these damage functions **only from an
> in-mission state** (e.g. `load_state 10`). The `0x8007xxxx`/`0x8009xxxx` code is
> swapped per overlay context — disassembling them from a menu/garage state shows a
> *different* resident overlay (a jump-table dispatcher), which earlier looked like
> the addresses were "offset." They are correct in-mission. All §8 finds were
> re-verified live on the Farsi build; combat code is base-game and byte-identical
> to the stock disc.

### 8a. Player-defense PRODUCER  CONFIRMED (live write-watch trap)
`0x80041214/15` are an **in-mission runtime global** (`0x0000` in the shop/menu),
filled at **mission load** by a derived-stats builder at **`~0x8009D8xx … 0x8009DA4C`**.
Trapped by write-watching `0x80041214` and driving slot 1 → arena. The math:
```
; integer SUM across equipped parts
lbu  partID, 0(a0)                       ; equipped-part id (id list @ 0x80001A58..)
lhu  s0, [0x800b9222 + id*stride]        ; part-table-A defense field
lhu  v0, [0x800b9e22 + ...]  ; addu s0,s0,v0   ; += part B
lhu  v0, [0x800b9522 + ...]  ; addu s0,s0,v0   ; += part C
lhu  v1, [0x800b947a + ...]  ; (cond) addu s0,s0,v1 ; += optional part
jal  0x800b63dc (a0=s0)                  ; int sum -> double
;  ... softfloat chain: × ~1.15, ÷, consts 256.0 / 45.0 / ~pi(3.140625) ...
;  clamp to ~[0,45]  (cmp vs 0.0 and 45.0 via 0x800b65bc/0x800b6770)
jal  0x80026f94                          ; double -> int (trunc)
sb   v0, 0x20(fp)   ; fp=0x800411F4 -> 0x80041214   ; shell-def byte (energy = parallel block -> +0x21)
```
**Are part defenses summed across the AC? Partly confirmed:** there IS an integer
accumulation (`addu s0,s0,v0` ×3–4) of per-part fields read from per-category tables
in `0x800B9xxx` (indexed by the equipped-part-ID list at `0x80001A58+`), feeding a
fixed softfloat curve (consts `1.15 / 256.0 / 45.0 / π`) → clamp ~`[0,45]` → trunc →
byte. Builder runs at mission-load (not the garage). Live: shell→`27`, energy→`32`.

> ⚠️ **UNRESOLVED — curve direction (the part that actually answers "how does it
> work"):** the runtime (`0x80075280`) makes `dmg ∝ attack × def_byte` — a *bigger*
> byte means *more* damage taken (the byte is a damage-transmission/vulnerability
> factor, **lower = tankier**). So if the producer were a plain bigger-is-bigger sum
> of part defenses, better armor would yield a bigger byte ⇒ MORE damage — absurd.
> Reconciliation must be one of:
>   1. the summed fields are a high=good defense rating and the float curve is
>      **decreasing** (e.g. `byte ≈ K − rating` / `K / rating`, clamped to [0,45]),
>      so good armor → low byte → low damage; **or**
>   2. the fields being summed are NOT protective defense (weight/other) and the real
>      def contribution is elsewhere.
> NOT YET VERIFIED which. Prior wording here ("summed → tankier") was an over-claim:
> the integer sum + float curve is observed, but the curve's **sign** was never
> decoded, and that sign is the whole answer.
> **To settle (cheap, live):** bump one part's source field in `0x800B9xxx`,
> reload→arena, read the output byte — ↓ ⇒ case 1, ↑ ⇒ case 2. Then decode the
> `0x800b5xxx`/`0x800b6xxx` softfloat ops statically (ideally on the backup disc) for
> the exact closed form, plus which table is head/core/arms/legs.

### 8b. Player-vs-enemy DISPATCH  CONFIRMED (live disasm)
The damage-apply `0x80075350` selects the defense path by a **hardcoded pointer
compare against the player AC** (array slot 0):
```
0x80075374  lui   v0, 0x801a
0x80075378  addiu v0, v0, 0x26B8     ; v0 = 0x801A26B8 = player AC
0x8007537C  bne   s0, v0, enemy      ; ac != player -> enemy path
0x80075384  jal   0x80075280         ; player path: part-derived def, dmg = ret<<1
            (enemy) lw v0,0(ac); lhu a0,0x14(v0); jal 0x800752f4   ; tmpl[0x14] nibble, dmg = ret
```
Not a flag, not a team field — the literal address `0x801A26B8` is baked in. **Every
non-player AC (named Ravens with full builds included) uses the `tmpl[0x14]` nibble
path; only AC slot 0 (the player) ever gets the part-summed defense.** Copying a
build onto an enemy would not change its toughness without editing that nibble.

## 7. Radar + lock-on (the shared target list)  CONFIRMED
Radar and lock-on are driven by **one structure: the target list at `0x80041020`**
— `0x80041020` = header (= `0x80041024`), then **8 slots × `0x30` bytes** starting
at `0x80041024`. Per slot:

| off | meaning |
| --- | --- |
| `+0x00` | **target object pointer** (an AC; 0 = empty slot) |
| `+0x04` | range / lock value (e.g. `0x1869`) |
| `+0x0C..+0x24` | projected/relative coords — **radar-blip position, screen pos, aim vector** |
| `+0x16` | flags: `0x1B` = active+lockable, `0x03` = empty |

- **Scanner `FUN_8007BA98`** (huge stack, GTE projection): iterates objects each
  frame, projects them, and fills the 8 slots (s4=`0x80041020`, s2=`0x80041024`).
  **`FUN_8007BA2C`** clears/inits the 8 slots first. → this is the **radar data**;
  the radar HUD draws a blip per non-empty slot from its `+0x0C..` coords.
- **Lock-on target = slot 0's AC = `*0x80041024`** (CONFIRMED: = the engaged enemy;
  read by the fire handler `0x8007CCC8`, the missile homing, and the target scope).

### The scan + projection math  CONFIRMED (Ghidra resident + m2c overlay)
The list is built by **`FUN_80073F38`** (the radar/targeting scan; m2c'd from the
live mission overlay). Per frame:
```
for each of 16 AC slots (base 0x801A26B8, stride 0x170):
    if AC active AND (AC.team[+0x2C]&7) != myTeam AND not excluded[+0x2E]:
       if visibility_bitmask[0x801A5DD8 + ...] & (1<<id):     # on-screen / valid
          d = AC.pos(int16 @+0x08/+0x0A/+0x12adj) - player.pos # (dx,dy,dz)
          if |dx|,|dy|,|dz| <= range AND dx²+dz² <= range²:    # bbox + radar circle
             slot.ptr  = AC
             slot.dist = SquareRoot0(dx²+dy²+dz²)              # 3D distance
             FUN_80014AE4(dx,dz,dy) -> (azimuth, elevation)    # angles to target
             azimuth  = (azimuth - playerYaw) & 0xFFF; wrap to [-2048,2048]
             if azimuth in [coneHmin,coneHmax]:
                elevation likewise; if in [coneVmin,coneVmax]:
                   slot.flags |= 2          # IN TARGETING CONE => lockable
             slot += 0x30; count++
    return count
```

**Angle/length primitives (all Ghidra-confirmed resident):**
- `SquareRoot0` (`0x80027324`) — GTE leading-zero + table integer sqrt.
- `ratan2` (`0x80028B64`) — Psy-Q table arctangent; **angles are 4096 units / full
  turn** (`0x400` = 90°), quadrant-corrected.
- `FUN_80014AE4` (`0x80014AE4`) — turns a 3D delta into **azimuth (`out+0`)** and
  **elevation (`out+2`)** via `ratan2` + `SquareRoot0`, masked `& 0xFFF`.

So **the radar is polar**: every same-frame enemy within range becomes
`(distance, azimuth, elevation)` relative to the player's heading; the HUD plots the
blip from the azimuth, and the **lock-on cone is just an angular window** (the
weapon's FOV) on the same numbers.

### Lock-on acquire logic  CONFIRMED (static, `~0x8007E9D0`)
Each frame the targeting update:
1. `jal 0x8007BA2C` (reset list) → finds the reticle candidate;
2. `sw zero,0x1024` — **clears the lock first** (so a poked value is wiped next
   frame unless re-acquired — explains why forcing `0x80041024` doesn't stick);
3. if a candidate exists, validates it: **flags `cand[0x16] & 0x0002`** (lockable)
   **and** the target pointer must be within the **AC array `[0x801A26B8 ..
   0x801A7C48]`** (24 × `0x170` AC slots) — only ACs are lockable;
4. `sw v1,0x1024` — sets the locked target; copies its position to **`0x80041034`**
   (the aim point used by the reticle / scope / missile guidance).

### Lock is a distance-timed fill (the interesting bit)  CONFIRMED (m2c `FUN_8007BA98`)
The reticle/lock update (`FUN_8007BA98`) doesn't lock instantly. Once the target is
inside the firing window (flags `&2` set by the scan, plus a tighter azimuth/elev
window `±arg1C/±arg1E`):
- it computes a **per-target lock threshold** ≈
  `(SquareRoot0(dx²+dy²+dz²) * weaponLockSpeed[+0x20] >> 8) * range[0x80041028] >> 15`
  — i.e. **farther target ⇒ bigger threshold ⇒ slower lock**;
- a **lock timer `0x8004102E` increments by 1 each frame** while held on target;
- the **reticle box stage** drawn (`FUN_80015258`, stage `0..8` = `0x8004102E>>2`,
  capped 8) is the shrinking-bracket animation;
- when `0x8004102E >= threshold` it sets **flags `|8` = LOCKED** and plays the
  lock jingle `FUN_8007BEB8(0x2A)`. Losing the target resets the timer to 0.

This is exactly why an idle target never locked for me: the player must keep the
target in the firing window long enough for the (distance-scaled) timer to fill.
**Lock flag bits in `0x8004103A`/slot `+0x16`: `2`=in cone/lockable, `4`=in firing
window, `8`=locked.** Primary lock value/range cache at `0x80041028`; aim point at
`0x80041034`; lock timer `0x8004102E`; threshold scratch `0x8004104A`.

Lock-target consumers (static scan for `lw …,0x1024`): `0x8007A658`, `0x8007ACDC`,
`0x8007CCC8` (fire), `0x8007D274`, `0x8007D9BC`. Clear-lock writers (`sw zero`):
`0x8007C960`, `0x8007DAB4`, `0x8007E9F8`.

> **Tooling note:** this DuckStation build's **read-watchpoints don't fire**
> (verified: read-watch on the per-frame-read player position got 0 hits). Use
> write-watch / execute-bp, or static instruction scans of a RAM dump
> (`(word & 0xFC00FFFF) == 0x8C00xxxx` for `lw`/`0xAC00xxxx` for `sw` with a known
> global offset) to enumerate readers/writers. `search_memory` is also unreliable.

## 6. AC array, the per-AC method table, and the ENEMY-AI FREEZE  CONFIRMED (live)
ACs (player + NPCs) live in their own array: **base `0x801A26B8` (idx0 = player),
stride `0x170`**. Each AC: `+0x00/+0x04` = its own template/geom-table slot ptrs
(per-AC, so the *player's exact header is unique* — don't search for it to find
NPCs); `+0x08`=X, `+0x0C`=Z, `+0x10`≈Y; velocity/orientation around `+0x64`.

**Inline method table at `+0x4C..+0x5C` (5 fn pointers)** — the AC "vtable",
invoked each frame by the per-AC processing passes (`lw vN,0xNN(ac); if vN!=0
jalr vN(a0=ac)` — every call is null-checked):

| slot | player (idx0) | enemy (idx1/2) | role |
| --- | --- | --- | --- |
| `+0x4C` | `0x80079888` | `0x800760DC` | main per-frame update wrapper |
| `+0x50` | `0x8007EF94` (pad ctrl) | `0x801C981C` | think |
| `+0x54` | `0x8008080C` | `0x801C75BC` | **movement / behaviour state-machine** (CONFIRMED) |
| `+0x58` | `0x80080D00` | `0x801C8CBC` | **weapon / fire AI** (CONFIRMED) |
| `+0x5C` | `0x8007DB98` | `0x801C91CC` | targeting / other AI |

The enemy update wrapper `0x800760DC` = `jal 0x80075FC0` (pre/anim) → `jal
0x8004F024` (**AC movement integrator**: applies the velocity/matrix at `ac+0x64`
to position `ac+0x08` via per-axis helper `0x8004EF74`; also answers §3) → `jalr
ac+0x50` (think). The NPC "brain" (targeting / navigate / decide-to-fire) is in the
mission-overlay routines `0x801C75BC` / `0x801C8CBC` / `0x801C91CC` / `0x801C981C`.

### ENEMY-AI FREEZE SWITCH  CONFIRMED & TESTED
**Zero an enemy AC's method table `+0x4C..+0x5C` (20 bytes)** → the per-AC passes
skip it (null check) → the NPC is **fully inert**: no movement, no aiming, no
firing. Verified live: enemy position byte-identical across a live run, no muzzle
flashes, player AP unchanged. Reversible by `load_state` (the table is restored).

```
enemy idx1 @0x801A2828:  write 20 zero bytes at 0x801A2874   (+0x4C..+0x5C)
enemy idx2 @0x801A2998:  write 20 zero bytes at 0x801A29E4
```
- **Full freeze** (statue): zero all 5 (`+0x4C..+0x5C`).
- **Move but NEVER fire** (CONFIRMED & TESTED): zero **only `+0x58`** (the weapon
  AI `0x801C8CBC`). Verified: over ~30 s the enemy navigated right up to the player
  but fired 0 rounds (spawn never called, player AP unchanged). Per enemy:
  idx1 `0x801A2880`, idx2 `0x801A29F0`.
- **Stop chasing / hold position**: zero `+0x54` (movement state-machine
  `0x801C75BC`) — leaves weapon/targeting (untested combo).
- `+0x50` alone does nothing useful (it's a minor think method).
- Use for **controlled-fire tests** and **buffet-free map navigation**. A permanent
  patch = a cheat holding the chosen method word(s) at 0, or NOP the enemy fire
  executor (below).

### Enemy fire path (CONFIRMED, separate from the player's)
`weapon AI +0x58 (0x801C8CBC)` → … → **enemy fire executor `~0x80077Dxx`** (builds
spawn args from a weapon-params struct: range `0x2BC`, dmg `0x1E`, etc.) →
`jal 0x80085E98` (shared projectile spawn, §4). This is a *different* executor from
the player's fire handler (`0x8007CCxx`), so enemy fire can be killed without
touching player fire — either by zeroing `+0x58` (cleanest, RAM-only) or NOPing the
`jal 0x80085E98` inside `0x80077Dxx`.

### Behaviour state-machine `0x801C75BC` (the `+0x54` method)  CONFIRMED
Reads the **AI state byte at `ac+0x41`** (passed in as `a1 = ac[0x41]` by the
dispatch loop) and switches on it:

| state | handler | notes |
| --- | --- | --- |
| 0 | `0x801C78CC` | (active; observed still moves+fires) |
| 1–3 | `0x801C768C` (default) | indexes a per-`ac[0x42]` sub-table (`88*ac[0x42]+216`) |
| 4 | `0x801C7658` | resets working vars (`ac[0x3c/0x3d/0x40/0xce]`) |
| 5–7 | `0x801C7AF8` | observed = **attacking** (fires) |
| 8 | `0x801C76E4` | observed = **less aggressive** (hangs back, ~no damage) |
| 9 | `0x801C7798` | |

Other AI fields on the AC: **`ac+0x48` = current target pointer** (= player AC
`0x801A26B8`); `ac+0x42` = sub-state index; `ac+0x3c/0x3d/0x40/0xce/0x136` = AI
working vars. In combat the enemy naturally cycles states **1 ↔ 2 ↔ 5**.

**KEY RESULT — internal AI vars can't be clamped:** freezing `ac+0x41` (state) or
`ac+0x48` (target) does **not** force behaviour — the AI **re-derives both every
frame** (target re-acquired to the player instantly; state recomputed inside the
method after it's read). Forcing state 8 only nudged aggression down. Therefore the
**reliable control surface is the method-pointer table** (`+0x4C..+0x5C`), not the
state/target bytes. A genuine "force idle/flee" needs **patching the state-write
sites** inside `0x801C75BC`/its handlers (NOP the `sb …,0x41(ac)` transitions after
setting the desired state), not a RAM freeze.

### Open / next targets
0. **Player-defense producer + dispatch condition** — **RESOLVED 2026-06-10**
   (live write-watch trap, Farsi build). See **§8a** (producer) and **§8b** (dispatch).
   - (a) PARTLY: `0x80041214/15` filled at mission-load by `~0x8009D8xx…0x8009DA4C`;
     per-part fields are integer-summed then float-scaled + clamped to a byte. **Curve
     sign UNRESOLVED** — runtime is `dmg ∝ def_byte` (lower=tankier), so the producer
     must invert the sum; not yet verified (see §8a ⚠️ box).
   - (b) DONE: `0x80075350` selects via a **hardcoded `bne s0, 0x801A26B8`** — every
     non-player AC uses the `tmpl[0x14]` nibble; only player slot 0 gets part defense.
   - Minor follow-up (statically, ideally on the backup disc): exact closed form of
     the float scaling curve + which `0x800B9xxx` table is head/core/arms/legs.
1. To force a behaviour state: find & NOP the `sb v,0x41(s1)` transition writes in
   `0x801C75BC` + handlers, then set `ac+0x41` once. Map states 0/8/9 handlers
   (`0x801C78CC`/`76E4`/`7798`) to behaviours (idle/evade/retreat).
2. **AC physics detail** (§3/§6): the `0x8004F024`/`0x8004EF74` integrator math,
   boost (energy gauge) and gravity terms.
3. `0x80056B54` (missile turn helper) + `0x8005C904` for the exact missile
   turn-rate; per-missile-type via the turn tables (`0x80039C7C[type<<3]`) — the
   WM-S40/1 is one entry; other missiles differ here.
4. Map weapon-def (`0x8008E530`, stride 0x1C) → think via `FUN_80085764`:
   rifle → `0x800851A8`, missile → `0x8008523C`.

---

## Tooling used
- `tools/` + ad-hoc python diffs of full `dump_ram` snapshots
  (`/tmp/ac1_rest.bin`, `ac1_move1/2.bin`, `ac1_pre/post.bin`).
- Method that worked best: **dump full RAM at controlled frame-stepped states and
  diff in python** (the emulator's iterative memory_scan was unreliable across
  `load_state`). Monotonic-delta filtering + object-array slot diffing.
