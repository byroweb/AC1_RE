# AC1 perception model — acquire → remember → lose (stealth-mod keystone)

Target **SLUS-01323 (v1.1)**. Static RE, 2026-10-19: offline `objdump` of the on-disc
FDAT 202 overlay (`disc_map/overlays/ovl202_mission.bin`, base `0x8004ADA0`; byte probe
`0x80083894 = afb70014` matches the mission build), the carved exe, and all 47 FDAT `2N`
mission code units (base `0x801C4B40`). `CONFIRMED` = read in disasm. `HYPOTHESIS` = inferred.

**Why this doc exists:** sensor modules, sentry mode, stealth infiltration and covered
mech-jacking all need detection to depend on line of sight. They all hang off the one
system described here. Change it once and every enemy picks it up.

---

## 1. Stock behaviour — what detection gates on today

### 1a. Target ACQUISITION `0x80073714` — `acquire(self a0, range a1)`  CONFIRMED
Shared overlay function. It is called **directly by every mission's AI code** (88 call
sites across the 47 units), never by the overlay itself. It writes the winner to
`self+0x48` and returns `dist*4`, or `-1` when nothing qualifies.

```c
myFac = self->+0x36 & 7;  best = 0;  bestScore = 0x7FFFFFFF;
for (slot = 0; slot < 16; slot++) {                 // 16 iterations (not 15)
    cand = 0x801A26B8 + slot*0x170;
    if (*cand == 0) continue;                       // empty / dead
    if ((cand->+0x36 & 7) == myFac) continue;       // ally
    if (slot == 0 && (*0x8019F524 & 0x400)) continue; // player-untargetable bit
    d = SquareRoot0(((dx>>1)^2) + ((dz>>1)^2));     // XZ only (+0x08,+0x0C); jal 0x80027324 @ 0x800737F0
    if (!(d < (range >> 2))) continue;              // the ONLY spatial gate
    // crowding: allies = #same-faction (excl. self), n = 1 + #allies already targeting cand
    score = (allies >= 2) ? d * n / allies : d;
    if (score < bestScore) { bestScore = score; best = cand; bestD = d; }
}
self->+0x48 = best;
```

- **No LOS, no FOV, no raycast here.** CONFIRMED. The engine *does* ship a working LOS
  test (§1e). Acquisition just never calls it.
- **The range is effectively off.** 77 of 88 call sites pass `range = 0x7FFFFFFF`, and the
  other 11 pass `self->+0xFC` (u16). So stock acquisition only checks that the target
  exists and isn't an ally. This matches "enemies track through walls". CONFIRMED.
- The player is "slot 0" **by address** (`bne s1,t1` against the array base), not by a flag.

### 1b. Aim/"LOS" check `0x80073C2C` — actually a range + FOV cone + area gate  CONFIRMED
Called on `self->+0x48` by the sub-AIs (red AC `0x801C59F8`). Despite the old label
(`ENTITY_AI_FRAMEWORK.md` §5), **it does not test walls**. In order:
1. Rejects a null, dead or same-faction target.
2. **Active-set gate:** `bit(target->+0x16)` in the 512-bit set at `0x801A5DD8` (§2). If the bit is clear, returns 0.
3. **Jam dropout:** `if (target->+0x38 & mask) && rand() < 0x4000` → return 0 (50%).
   `mask` is the first stack argument; at least 20 of the 127 call sites pass `8`
   (29 sites build their arguments in a way the quick scan didn't resolve).
4. Box test, then sphere test against `range` (callers pass 3000–15000).
5. Azimuth/elevation from `0x80014AE4`, checked against a caller-supplied FOV window
   (e.g. red AC: yaw ±1024 = ±90°, pitch −128..+1024). Being in the cone sets `out+0x16 |= 2`.
   The function returns 1 when the target is in range, whether or not it is in the cone.

### 1c. Sensor/eye point `0x800745D0`  CONFIRMED
This is the **model-node world position** helper: `0x80055A24(self->+0x60, node args, pos +0x08, …)`.
It is not a range sensor (another relabel). Use it as the **enemy eye/muzzle origin** for a sightline.

### 1d. Firing
The red AC fires only when the sub-AI says the target is inside the FOV cone (bit `2`),
and then on an RNG roll (`rand < 2622`). FOV is enforced; walls are not.

### 1e. The stock LOS helpers — already shipped, used only for some fire branches  CONFIRMED
- **`0x8007816C los_ray(a, b, out)`** builds an int32 segment from `a`'s centre
  `(X, Y − (+0x2A)/2, Z)` to `b`'s centre. It calls the **world segment ray `0x800705D8(seg,
  out, mask 7)`**: scratchpad setup → bit-grid broadphase `0x8006FB6C` → per-section/face,
  nearest hit (best = `0x7FFFFFFF`). It returns the hit record (0 = clear).
- **`0x800781E8 has_los(self)`**: `t = self->+0x48`. If `t` is null or dead → 0. If no hit
  → 1. If hit → 1 only if the blocking section record (`hit->+4`) has `+0x06 == 0x30`.
  HYPOTHESIS: that marks a see-through class (fence/glass). Otherwise 0.
- **Usage:** 58 call sites across **37 of 47** mission units. The overlay calls `los_ray`
  only from `has_los`. In FDAT 16 (`0x801CB214`) it gates one case of a weapon-type
  switch: that fire branch needs LOS, the others don't. So stock = **partial LOS for
  firing, none for acquisition**, which matches the observed behaviour.
- `0x800705D8` is also the world ray used by projectiles (`0x80083208`, `0x80084188`,
  `0x80085C84/CDC/D50`, `0x80087CC8`) and by the blade hitscan `0x8007A334`. It's well
  exercised.
- **Live (P-1, 2026-10-19, mission 8):** `los_ray` works against world geometry.
  CONFIRMED (live). On a hit, `v0` = the hit-list entry `0x801E8B74` (the list from the
  `a3 = -1` row in §4's table). `hit+4` = a section record in the `0x801D0B68 + i*64`
  table (§2; the sample was index 140). The first 3 s16 of the `out` buffer are the hit
  point. Its first 16 bytes read as a bounding box `{minX, minY, minZ, flag, maxX, maxY,
  maxZ, pad}`; HYPOTHESIS, from one record. So `has_los`'s `+0x06 == 0x30` test reads the
  `flag` halfword. Every live hit had `flag = 0` (solid).
- **Stock `has_los` is rare in mission 8:** an exec breakpoint at `0x80078218` got
  **0 hits in 60 s** of combat. All four attackers there use think `0x801C5B7C`. So stock
  firing almost never consults LOS in this mission.

---

## 2. Existing free lever — the section "active set" (coarse PVS)  CONFIRMED (static)
- Every frame `0x8007922C` clears the 16-word set `0x801A5DD8` (`0x8006E7B8`). It then ORs
  in the **player's** current section (`slot0+0x16`), plus that section's list
  (`0x8006E7D4`: section record `0x801D0B68+i*64`, `+0x26` = count, `+0x28` = u16 list;
  `0x8000` = recurse, low 9 bits = section index).
- Every entity's `+0x16` is its current section. The generic updater re-derives it each
  frame (`0x800789CC`: `+0x16 = 0x8006E6B4(+0x16, &pos)`).
- Consumers: the cone test (§1b), the player lock/radar scanner `0x80073F38`, the proximity
  spawner `0x80078F00`, and the despawner `~0x800791E4`.

**Implication:** a near-free, room-level stealth gate already exists in data. "Enemy
section ∉ player's active set" ⇒ the enemy is in a room that isn't visible from the
player's room. It costs one bit test. HYPOTHESIS: the per-section lists are
visibility/adjacency, roughly symmetric. That's the authoring knob for "rooms".

---

## 3. Target model (the system to build)

```
            acquire (gated)                 LOS lost
 UNAWARE ───────────────▶ TRACKING ───────────────▶ SEARCHING (memory timer)
    ▲    sightline+FOV+range    │                       │  timer→0
    │                           │ fire/aim as stock      ▼
    └──────────────────────────────────────────── UNAWARE  (+0x48 = 0)
 damage taken (retarget writer, §5) ──▶ TRACKING (alerted by noise/hit)
```

- **Acquire:** candidate is the player (slot 0) **and** within range **and** inside a FOV
  window around `self+0x12` yaw **and** has a clear sightline. Sightline = the stock
  `los_ray 0x8007816C(self, player, out)` (centre to centre), with the same
  `+0x06 == 0x30` see-through rule as `has_los`.
- **Remember — via a ghost target, not by keeping `+0x48 = player`.** `+0x48` is an
  entity pointer, and the sub-AIs read the target's *live* position through it. Keeping it
  on the player for the memory window is therefore just omniscience. Instead, point `+0x48`
  at a **ghost entity** that holds the AI's belief (prototype live: P-5, §7). CONFIRMED
  (live).
  - **On every clear sighting:** copy the player's 0x170-byte record into the ghost. That
    gives the target-side fields the AI code reads valid values: `+0x00`, `+0x08..0C`,
    `+0x16`, `+0x36`, `+0x38`, `+0x6C`. Record the position and the frame, and set
    `expiry[slot] = now + MEM`.
  - **Velocity:** re-measure it only over a baseline of ≥ 8 ticks, using last-seen
    positions, never the live player. Sightings from different AIs arrive 1–2 ticks apart,
    and instantaneous deltas were noise (32 vs a real ~30, and a −1078 spike from a zero
    baseline).
  - **Every game tick while hidden:** `ghost += v`, `v -= v >> DECAY`, stopping after `CAP`
    ticks. The ghost glides along your last heading, slows down and parks. That is "where
    it expects you to pop out". The enemy aims and moves at it, so a player who breaks the
    line (doubles back, goes round the other side) beats the prediction.
  - **In `acquire`'s tail:** if nothing qualified (`best == 0`) and the AI's memory hasn't
    expired, store the ghost instead of 0.
- **Lose:** after `MEM` ticks with no sighting, `acquire` stores 0 (UNAWARE). An NPC
  candidate, if there is one, still wins as stock.
- **Ticks, not vblanks:** the per-frame driver runs at **~22 Hz** in mission 8 (365 game
  ticks per 1000 vblanks). Express `MEM`/`CAP` in ticks: `MEM 100` ≈ 4.5 s, `MEM 300`
  ≈ 13.7 s.
- **Shared vs per-AI belief.** P-5 kept **one shared ghost** ("squad radio"), 368 B, with
  every enemy hunting the same spot. The side effect is that while *any* AI sees you, the
  ghost sits on you, so hidden AIs still hunt your real position.
  - **P-6 per-AI ghosts (current design):** a pool of 4 ghosts, each owned by one AI. An AI
    claims one on its first own sighting, and only its own sightings update it. When the
    owner's memory expires, the ghost is reclaimed. CONFIRMED (live): a blinded AI keeps
    its own stale belief while the AIs that can see you track you. More than 4 hunters at
    once → the extras get no memory and drop straight to UNAWARE. That's a sane fallback;
    see `allocfail`.
  - **Radio delay:** still an option on top, e.g. a spotter could push its sighting to the
    others' ghosts after N ticks.
- **Movement follows the target's position.** CONFIRMED (live, P-6 lure). The fliers'
  steering uses `+0x48`'s `+0x08/+0x0C`, so a ghost pulls them to where the AI believes
  you are. With a null target they leave the area entirely.
- NPC↔NPC targeting stays stock (omniscient). That's cheap and invisible to the player.

### Alert states as ghost placements (design target; P-7)
The movement code steers to the position behind `+0x48`. The aim helper (red AC
`0x801C59F8` → cone `0x80073C2C`) turns and fires only when the cone accepts that target,
and the cone rejects a target of the AI's own faction. So **one pointer and two ghost
fields** give a whole behaviour set: ghost position = where to go; ghost faction
(`+0x36 & 7`) = whether to shoot.

| state | ghost position | ghost faction | effect | status |
|---|---|---|---|---|
| UNAWARE / guard loop | patrol waypoints (authored, or a ring around the spawn) | AI's own | patrols, no aim/fire; today `+0x48 = 0` makes them leave | CONFIRMED (live, P-7; dwell ≥ 5 s per waypoint) |
| INVESTIGATE | noise source / hit point | AI's own | goes and looks, holds fire | mechanism CONFIRMED (own-faction lure); noise sources not hooked |
| SEARCHING | last-seen + decayed extrapolation, then a spiral around it | player's | hunts, shoots on contact | P-5/P-6 CONFIRMED; spiral not built |
| FLANK | predicted player + per-AI lateral offset | player's | spreads the squad, but aims at the offset point | PARTIAL (P-7). Needs **aim ≠ move**: the cone shim should pass the real player as the cone's `a0` while `+0x48` stays on the flank ghost |
| TRACKING | real player | — | stock combat | stock |

The orbit radius the fliers keep around their target (~4–5k in mission 8) sets the
smallest useful waypoint spacing and flank offset. HYPOTHESIS: ground classes differ.

### Null target is a supported state  CONFIRMED / HYPOTHESIS
`acquire` already writes `+0x48 = 0` when nothing qualifies (the 11 range-limited call
sites hit this routinely), and `0x80073C2C` returns 0 for a null target. So a gated miss
doesn't create a new state. HYPOTHESIS: every mission's movement handler tolerates
`+0x48 == 0` (implied by the range-limited variants). Verify per mission class.

---

## 4. Chosen insertion point  [NEEDS PATCH]

**Primary: one-word hook in `acquire` at `0x800737F0` (`jal 0x80027324` → `jal los_sqrt`).**
- `los_sqrt(a0 = dist²)` returns `SquareRoot0(a0)` unchanged unless `s0 == 0x801A26B8`
  (the candidate is the player). In that case it runs the gate (range/FOV/sightline/memory)
  and returns `0x7FFFFFFF` on failure. The existing `slt v0,t0,s5` then rejects the
  candidate. `s0` (cand) and `s2` (self) are live callee-saved registers, so no other
  bytes change.
- The sightline is a call to the stock `0x8007816C(s2, s0, &tmp)`, so no new collision
  code is needed. Its signature is exactly (self, candidate).
- **Covers all 47 missions at once**, because they all call this shared function.
- Cost: at most one world ray per AI per acquire call (only the player is ray-tested).
  That's the same primitive 37 missions already run from fire logic.
- **Live-proven RAM-only (P-1, 2026-10-19).** CONFIRMED (live). The hook works as described.
  In the live disasm at `0x800737F0`, `s0` = candidate, `s2` = self, and `a0` = dist² (set
  by the jal's delay slot `addu a0,a0,v1`). `t1` is spilled to `sp+0x10` around the call,
  so a stub may clobber any caller-saved register. When the stub returns `0x7FFFFFFF`,
  `acquire` drops the player and writes `+0x48 = 0` if no other enemy-faction candidate is
  left. Corrections from the live run:
  - **Call rate:** each AI calls `acquire` about 1–2×/s, with bursts of ~10–20/s
    (seen at 9–12k units). So the "one ray per call" cost is small.
  - **Memory:** without the §3 memory timer, `+0x48` follows the most recent ray, so it
    toggles PLAYER ↔ 0 at the edge of cover. The memory/decay state is needed for the
    feel, not for correctness.
  - **Recompiler:** DuckStation's recompiler picked up MCP `write_memory` code edits (to
    the overlay jal and to fresh code in `0x801CF5xx`) without switching to the
    interpreter.

  The working stub is ~59 instructions at `0x801CF500` (per-slot logging included). Its
  assembler and harness are kept outside the repo in the session notes folder
  (`p1-scripts/stub.py`).

**Secondary (optional, belt-and-braces):** hook `0x80073C2C`'s active-set test
(`0x80073CD0..0x80073CE4`) so aim/fire also needs LOS. Without it, an enemy that kept
`+0x48` from memory still aims through walls during the search window. That might
even be desirable.

**Code home:** ~~overlay-202 cave `0x800908AE..0x80090D9C`~~ — **REFUTED (live,
2026-10-19).** The range is zero on disc but holds **runtime data** in a live mission.
Mission 8 showed 243 non-zero bytes there: RGB palette/gradient tables from
`0x80090920`, a `0x40404000` word every `0x80` from `0x80090A2C`, the scratchpad pointer
`0x1F800020` at `0x800908D8`, and the on-disc checksum word at `0x80090D9C` overwritten
with 0. This is the zero-initialised `.data`/`.bss` case the farsi GOTCHA warns about.
Don't put code there.
**Use instead: the resident gap `0x801CF340..0x801D0B3F` (6144 B, §5).** The P-2 canary
held: every byte stayed `0xA5` through ~66 s of mission-8 combat and the mission-end
debrief. The P-1 stub ran from `0x801CF500` for >4 min with no crash. That proves it free
in mission 8 only; other missions are still HYPOTHESIS, since this gap sits above the
largest unit and smaller units leave more room. On disc it is not part of FDAT 202 (it's
past the overlay end `0x80090DA0`), so a shipped patch needs a loader that copies the stub
there at mission load, or an overlay-202 tail extension. Repack + checksum per
`MXT_LOADER.md` / `t_repack.replace_entry`. [NEEDS PATCH]

### Fallback sightline primitive `0x80070F30` (swept AABB) — not needed now
Only relevant if a box-shaped (fat) sightline is wanted. The segment ray above is the
better tool: int32 endpoints, so there's no s16 delta limit.

| item | value | status |
|---|---|---|
| a0 | AABB, 6×s16 `{x-r, y-h, z-r, x+r, y, z+r}` (built by wrapper `0x80071014` / `0x80074800`) | CONFIRMED (layout) |
| a1 | sweep delta 3×s16; the wrapper stores it **<<2** and divides results by 4 ⇒ ≤ 8191 world units per sweep → segment longer sightlines | CONFIRMED (encoding) / HYPOTHESIS (units) |
| a2 | result buffer (resolved pos) | CONFIRMED |
| a3 | mode; `-1` walks a hit list at `0x801E8B74` (stride 28) | HYPOTHESIS |
| `sp+0x10` | collision flag-group mask (`7` = all groups) | CONFIRMED |
| v0 | non-zero = hit | CONFIRMED |
| side effects | uses scratchpad `0x1F800000+` (`0x8006F4D8`) | CONFIRMED; it is already called from inside per-entity updates (movement), so calling it from think context should be safe (HYPOTHESIS) |

## 5. Suspicion / memory state — where it lives
- Need ≈ 16 slots × {u16 timer, s16 lastX, s16 lastZ, u8 level} ≈ 128 B.
- **Preferred (resident, outside the swap range):** the gap between the largest mission
  unit's end and the world page, **`0x801CF340..0x801D0B40` (~6 KB)**. The biggest unit
  (FDAT 16) is `0xA800` B from `0x801C4B40`. **CONFIRMED (live) free in mission 8**
  (= FDAT 16, the largest unit): the canary held and the P-1 stub ran there (§4, §7). All
  6144 B were zero at mission start (`find_free_ram` reports exactly
  `0x801CF340`/6144). HYPOTHESIS for the other missions.
- ~~Alternative: inside the overlay cave~~ — refuted: the cave is live runtime data (§4).
- Other `+0x48` writers to keep in mind: one per mission unit, e.g. FDAT 16 `0x801CA71C`
  (`sw v0,72(s1)` from a stack value). HYPOTHESIS: this is the on-hit/retaliation retarget.
  Keep it as the **"alerted by damage/noise"** channel, so don't gate it.

## 6. Other existing knobs (stealth-adjacent)
- `0x8019F524 & 0x400` — the player-untargetable bit. It is set by the chunk-4 tail bit
  `0x80000` (opcode `0x0013`, see `MISSION_DAMAGE_EVENTS.md` §4). It also **zeroes player
  input** (`0x80079524`) and is checked by the player fire/lock paths (`0x8007E0CC`,
  `0x8007E7E0`, `0x8007E884`). It's a cutscene/scripted-sequence flag, **not** usable as a
  stealth cloak.
- `target+0x38 & 8` → 50% detection dropout in callers that pass mask 8. The player's
  `+0x38` is seeded from `0x80041226 & ~3` in player init `0x8007ED84`. HYPOTHESIS: part
  capability bits. This is an existing "jammer" hook — a candidate base for a sensor/ECM
  module.

## 7. Open questions and live recipes (DuckStation)
- **P-1 — DONE 2026-10-19: CONFIRMED (live). LOS-gated acquisition works RAM-only.**
  DuckStation MCP fork (0.1-12000, Recompiler), no disc or file patches.
  - **Getting there.** "Mission 16" means **FDAT entry 16 = mission number 8, "Guard
    Freight Train"**. Save slot 1 no longer holds it: slots are scratch, and slot 1 is now
    the garage (`GAR-main-01`). A new game doesn't list mission 8, so force it. From the
    garage: MISSION → pick any mission. Set a write breakpoint on `0x800411F8`. The front
    end stores the mission number at `0x8008A890` (`sb v0`, from the table
    `0x80044314[menu idx]`). After that store, write `08` there and continue. The desert
    train map loads, and `0x8019F518 == 8`.
  - **Scene.** At mission start, slots 2–5 all run think `0x801C5B7C` with HP 1200, faction
    2, `+0x48` = player. They fly (Y down to −17000). Slot 1 has `*ent == 0` (HP 5450,
    think `0x801C6B10`), so `acquire` skips it. Slots 6–7 are empty. This differs from the
    slot map in `ENTITY_AI_FRAMEWORK.md`, which came from another phase or state. The
    attackers kill the player in ~35 s. `memory_watch freeze` did not hold HP, and cheats
    were unavailable. So both HP stores in the damage router (`0x800753C4`/`C8`) were
    NOPed, making every entity invulnerable. The `+0x5C` on-hit callback still runs.
  - **Step 2 (stock helper).** A breakpoint at `0x80078218` got 0 hits in 60 s (§1e), so
    the instrumented stub was used instead. Run first in **observe mode**, it logs and
    passes stock sqrt through. Over ~3 min of rays from all 4 AIs to the player: near rays
    (< ~5k units, open desert floor) came back **clear (`v0 == 0`) every time**. Blocked
    rays clustered at 6–13k with the AI behind the canyon walls (sections 13/16/17/36).
    One sample, for the ray A=(−4709,−2873,−11525) → B=(−9140,−107,1775), was checked
    in full. The hit point (−5034,−2671,−10550) is exactly on the segment (t ≈ 0.073). It
    lies inside section record `0x801D0B68[140]`'s box (x −6250..−5000, y −5000..−2500,
    z −11250..−8750), and that record's `+6` flag is 0 (solid). So the ray reports real
    world geometry. In observe mode `+0x48` stayed = player regardless (stock is
    omniscient, as §1a says).
  - **Steps 3–4 (gate on).** Stub at `0x801CF500`, jal at `0x800737F0` → `0x0C073D40`,
    MODE=1. Within the same poll window as a blocked ray (2/2 … 18/18 hits), **the AI's
    `+0x48` drops to 0**, and it returns to the player once a clear ray comes through.
    Examples: slot 3 at 10–13k in section 36, slot 5 at 13–26k in sections 17/20/21,
    slot 2 at 10–28k in sections 16–18 as the player walked east up onto the raised
    ground in section 19. This "slot 2" is the AI at `0x801A2998`, `+0x48` at
    `0x801A29E0`. Over a 60 s soak with the player moving: 406 player-candidate rays,
    27 target flips, `+0x48 == 0` in 16.7% of samples. **It retargets to 0, never to the
    train or aircraft:** no other faction-1 entity is acquirable in this phase (slot 1
    fails `*cand != 0`). **No crash, hang or exception** in >4 min of enforced play,
    including ~3600 frames under recompiler at full speed.
  - **Not covered.** The player didn't deliberately park behind a chosen structure. The
    cover changes came from the fliers' and the player's movement across the canyon. A
    screenshot-correlated "duck behind the rail tower" run is still a nice-to-have. Firing
    and movement behaviour while `+0x48 == 0` is still P-4.
- **P-5 — DONE 2026-10-19: ghost memory + dead-reckoning (§3 "Remember"). CONFIRMED
  (live), RAM-only, mission 8, same harness as P-1.**
  - **Hooks (7 RAM words):**
    - `0x800737F0` → the LOS gate + sighting update.
    - `0x800738C0/C4` (`sw s7,0x48(s2)` / `beq`) → `j ATAIL; nop`. ATAIL re-implements the
      store/return and substitutes the ghost. That's safe because `acquire` restores `ra`
      from its stack, so ATAIL may call `SquareRoot0` to return the ghost's distance in
      `acquire`'s units.
    - `0x800795A8` (`jal 0x8007922C` in the per-frame driver `0x8007959C`) → the tick
      stub, which tail-jumps to the active-set build.
    - The three FDAT 16 calls to the aim cone `0x80073C2C` (`0x801C5A74`, `0x801CAC6C`,
      `0x801CC0F0`) → a stack-neutral logging wrapper. Stack args are untouched; the out
      struct is the caller's `sp+44`.
  - **Code and data:** ~270 instructions at `0x801CF600`, ghost at `0x801CFC00`, globals
    and a 32 B/slot table at `0x801CF340`/`0x801CF400`. All of it is in the §5 gap.
  - **Static safety (ovl202 + live FDAT 16 scan):** all target-pointer accesses through
    `+0x48` are reads. The FDAT 16 AI reads `+0x08/+0x0A/+0x0C/+0x38`; the cone reads
    `+0x00/+0x16/+0x36/+0x38/+0x08..0C/+0x6C` and writes only its own out struct. The only
    `+0x48` writer in FDAT 16 is the retaliation store `0x801CA71C`. So a copied ghost is
    never written through. CONFIRMED (static scan, heuristic: a 25-instruction window per
    load).
  - **Blackout test.** A test knob forces every ray "blocked" while the player walks
    straight at ~30 units/tick. The result is the whole state machine:
    - **TRACKING → SEARCHING:** all 4 AIs switch `+0x48` P→ghost within one 0.5 s sample.
    - **Prediction:** the ghost extrapolates along +X, v 26.7 → 5.2 with `DECAY 6`, and
      parks after `CAP 120` ticks, ~1.4k units on.
    - **The AIs really aim at it:** while blind, aim-cone calls on the ghost returned "in
      range" 845/922 and "in FOV cone" 454/922. Before the blackout it was ~0, because the
      ghost was rarely the target.
    - **SEARCHING → UNAWARE:** with `MEM 100`, the memories expire one by one (s2 first),
      and all four `+0x48` reach 0 by ~117 ticks after the last sighting.
    - **→ TRACKING:** on clearing the knob, `+0x48` is back on P within one sample and the
      ghost snaps to within ~40 units.
  - **Real-cover soak:** 60 s, player walking, no knob. Targets were P 69 / G 11 / 0 of
    80 samples, with no crash or hang. Hidden AIs mostly got a ghost glued to the player,
    which is the shared-belief side effect noted in §3.
  - **Still open:** movement toward the ghost was answered in P-6. Still open:
    - Whether the AI fires at the ghost. That can't be judged with the HP stores NOPed;
      it's P-4-adjacent.
    - The behaviour when the ghost's copied section `+0x16` leaves the player's active
      set. The cone would reject it — test as a room-change "they lose you" rule.
    - Tuning `MEM`/`DECAY`/`CAP`, and the shared-vs-per-AI choice.
- **P-6 — DONE 2026-10-19: per-AI ghosts + "do they move toward it?" CONFIRMED (live),
  RAM-only, mission 8.**
  - **Build:** the stub is now **C + asm shims**, built with `mipsel-none-elf-gcc` 15.2
    (`-march=r3000 -mips1 -msoft-float -mno-abicalls -fno-pic -G0 -O2 -ffreestanding
    -nostdlib`) and linked at `0x801CF700` by a linker script that defines `los_ray` and
    `SquareRoot0` by absolute address.
  - **Shims:** they bridge the hook sites' non-ABI registers:
    - `sqrt_hook(s2, s0)` → `on_candidate`.
    - `store_hook(s2, s7)` → `on_store`, which re-does the `sw s7,0x48(s2)` and return
      tail and loads `fp` from `ret_dist`.
    - `tick_hook`, which saves `a0..a3`, calls `on_tick`, then `j 0x8007922C`.
    - The stack-neutral `cone_hook`.
  - **Size and layout:** 1744 B of code. Globals `0x801CF340`, slot table `0x801CF400`
    (32 B/slot), belief pool `0x801CF600` (4 × 48 B), ghosts `0x801D0400..0x801D09C0`.
    All inside the §5 gap.
  - **Hooks:** the same 7 words as P-5.
  - **Method:** each condition starts from the same mission-8 start state. The player walks
    +X for 25 s, and a per-slot `blindmask` is applied from t = 4 s to t = 16 s. That's
    `MEM 300`, `DECAY 6`, `CAP 120` (ticks).
    The seen phase is near-deterministic across runs: AI positions stock vs ghost differ
    by ≤ 150 units.
  - **Per-AI divergence (blind slot 3 only):**
    - Slot 3 kept **its own ghost** parked at x ≈ −4543, last-heading extrapolation then
      `CAP`, while the player walked on to x = +3183.
    - Slots 2/4/5 kept `+0x48 = P`, and their own ghosts tracked the player within ~50
      units.
    - Slot 3's memory ran 300 → 0 over ~12.5 s, then `+0x48 = 0`. No allocation failures
      or evictions.
  - **Movement**, median XZ distance per AI over the late blind window (t 20–32):

    | condition | `+0x48` | AI centroid | d(AI, player) | d(AI, lure) |
    |---|---|---|---|---|
    | stock (no blind) | P (some G on real cover) | (−2665, 1659) | 5.5k | 12.0k |
    | ghost (all blind) | own G, parked near (−4800, 1775) | (−2841, 1587) | 5.4k | 11.7k |
    | **lure** (all blind, every ghost pinned at (−13000, 6000)) | own G | **(−13302, 5861)** | 9.7k | **4.6k** |
    | null (MODE 1, all blind) | 0 | (−9005, 13905) | 25.6k | 19.7k |

  - **Reading the table:**
    - The fliers **steer to their target's position.** Under the lure their centroid moved
      ~10k onto the pinned point, and they orbited it at the same ~4–5k radius they hold
      around the player in stock. Their tracks circle it (e.g. slot 2 (−13193, 5671) →
      (−13629, 10311) → (−9090, 10854) → (−13212, 5208)).
    - "ghost" looks like stock only because the parked ghost was within ~5k of the walking
      player.
    - **With `+0x48 = 0` they leave**: 18–29k away by the end. So the memory/ghost stage
      also decides whether a lost enemy hangs around and searches or wanders off.
    - The aim cone accepted the ghost all through the blind phase: in range 1096/1177
      calls, in the FOV cone 449/1177.
  - **Intermittent stall, explained:** in some all-blind runs the player stops dead at
    exactly x = −4489 and stays there.
    - It reproduced in a later run that had no pauses and no other MCP client, so it is
      not the peer-session `initialize`, which I first suspected.
    - The capture shows the player taking hits ("DAMAGED", missile impacts) with no enemy
      nearby. The ghosts park at the last-seen spot, x ≈ −4470..−4540. The enemies are
      shelling their ghosts, the player walks into that area, and is pinned there.
      HYPOTHESIS: by repeated hit-stagger; HP was locked for the test.
    - It's intermittent because it depends on whether shots land as the player arrives.
    - It's a real behaviour, suppressive fire on the last-known position, rather than a
      hook bug.
  - **Still open:**
    - Fire at the ghost (needs HP stores live: P-4).
    - The section `+0x16` / active-set interaction when you change rooms.
    - Ghost-pool sizing in missions with > 4 attackers.
    - A non-flying AI class (ground MT/AC) and whether it pathfinds to the ghost or
      beelines.
- **P-7 — DONE 2026-10-19: ghost-driven behaviour states (patrol / investigate / flank).
  Mission 8, P-6 build.** The harness places each AI's own ghost every 0.5 s during the
  blind window. These are host-side writes to the belief and the ghost record; no new
  stub code.
  - **Static:** the red-AC aim helper `0x801C59F8` calls the cone (range 10000, yaw window
    ±1024, pitch −128..1024). When the target is in the cone, it sets the aim at
    `+0x76/+0x78` via `0x80015258` and returns 1; otherwise it resets the aim and returns 0.
    The cone rejects a same-faction target at `0x80073CB0`.
  - **Own-faction lure** (all ghosts pinned at (−13000, 6000) with `+0x36` faction = 2):
    late-blind centroid (−11260, 4782), median d(lure) **4.7k**. A hostile lure gave
    4.6k. Cone on the ghost: **0 of 1046** in range (the hostile lure gave 879 of 1043).
    **CONFIRMED (live): they steer to it and never aim or fire at it.**
  - **Patrol** (ring (2000,2000) → (−6000,10000) → (−14000,2000) → (−6000,−6000), 3 s per
    leg, own faction): in every leg the 4-AI centroid moved toward the new waypoint, from
    (−3238, 1299) to (−854, 5322) to (−8957, 4578) to (−10434, −3187). It ended 5–7k
    short, because 3 s legs plus the ~4.5k orbit don't give enough time to arrive. Cone
    on the ghost: 0 of 1043. **CONFIRMED (sequence-following); use ≥ 5 s per leg.**
  - **Flank** (ghost = live player ± 6000 in Z for s2/s3, 0 for s4/s5; player faction).
    Mean Z offset from the player in late blind, compared with stock:

    | AI | target offset | flank run | stock |
    |---|---|---|---|
    | s2 | +6000 | +5198 | +2542 |
    | s3 | −6000 | −4212 | −4772 |
    | s4 | 0 | −1358 | +914 |
    | s5 | 0 | +1380 | +283 |

    So s2 moved its side as told, and s3 was already there. The cone took the flank ghosts
    (1009 of 1039 in range), meaning they **aim at the offset point, not at you**.
    **PARTIAL.** To finish it, split aim from move in `cone_hook`, passing the player's
    record as the cone's target while `+0x48` stays on the flank ghost. Then retest with
    offsets larger than the orbit radius.
- **World scale — OPEN (two rulers disagree by ~2.3×).** Mission 8, live 2026-10-19.
  - **Hitbox sizes** (`+0x2A` height, `+0x6A` horizontal half-extent):

    | entity | height `+0x2A` | half-extent `+0x6A` | notes |
    |---|---|---|---|
    | player AC | 212 | 75–100 | `+0x6A` varies with state |
    | enemy fliers (slots 2–5, think `0x801C5B7C`) | 150 | 350 | |
    | friendly tilt-rotor transport (slot 1, think `0x801C6B10`) | 375 | 750 | HP 20000; carries the AC in at (−21840, −2971, 1775) and drops it — not the train |

  - **Ruler A (AC):** if an AC is 7–8 m tall and `+0x2A` is its visual height, 1 unit ≈
    3.3–3.8 cm. Flier cruise (a constant 3,610 units/s) ≈ 270–305 mph; walking (742
    units/s) ≈ 90–100 km/h.
  - **Ruler B (train):** the parked freight train is level geometry, not an entity.
    - It's in the section table `0x801D0B68` as records #175–#179 with flag `0x20/0x21`:
      a 937-unit leading box, then **4 identical cars, 1250 long, 219 wide, top 268
      above the rail**, end to end at x ≈ 20,100–26,100, z ≈ 0.
    - The rail-bed boxes are 187 wide.
    - Against a US freight car (~15–18 m long, ~3.2 m wide, ~4.6 m tall) and ~2.6 m
      sleepers, that gives **1 unit ≈ 1.4–1.7 cm**. Under that ruler the AC is ~3–3.6 m
      tall, the fliers ~110–140 mph, and walking ~40 km/h.
  - **How to resolve it:** compare the AC and train *model* vertex bounds (render data),
    not hitboxes. The train sits beyond the mission-8 area boundary (x ≈ 14,400–15,100,
    "LEAVING AREA" → mission end), so a side-by-side screenshot isn't possible there.
- **P-2 — cave/free-RAM canary. DONE for mission 8:** the cave is refuted and the gap
  `0x801CF340..0x801D0B3F` is free (§4/§5). Still to do for other missions: repeat the gap
  fill+diff in a smaller-unit mission and across a cutscene/phase change. (Read-watchpoints
  don't fire on this DuckStation build, so use fill+diff rather than watches.)
- **P-3 — active-set symmetry.** Dump `0x801A5DD8` (64 B) while standing in rooms A and B.
  Check that A∈set(B) ⇔ B∈set(A) for the sections you'll author.
- **P-4 — null-target tolerance.** RAM-only: in the red-AC scene, write `+0x48 = 0` on
  slot 2 every frame (script loop). Expect no fire and no crash; note the movement fallback.
