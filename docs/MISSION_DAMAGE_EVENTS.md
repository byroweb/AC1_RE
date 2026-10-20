# AC1 mission damage events — "deal damage in a region on a flag"

Target **SLUS-01323 (v1.1)**. Static RE, 2026-10-19: offline `objdump` of the on-disc
FDAT 202 overlay (base `0x8004ADA0`) and the FDAT `2N` mission units. `CONFIRMED` = read
in disasm. `HYPOTHESIS` = inferred. Use cases: airstrike, breach charges, mech-breach
defenses.

**Verdict:** the damage itself is **[NEEDS NEW CODE] (small)**. Defenders spawning on a
breach are **[DATA-ONLY]**. `op_f` can't deal damage.

---

## 1. `op_f` (0x000F) is a cosmetic FX emitter, not a projectile  CONFIRMED
- Dispatch `0x8008B6B0`: stores the record offset in thread scratch `+0x26` and skips
  `2 + cnt*4` bytes.
- Every opcode `< 0x20` also sets bit `1<<op` in the thread's "opcode-seen" mask
  (`0x8008B444..B454`). `op_f` sets `0x8000`.
- Tail (`0x8008BEE4..0x8008C000`) runs once per frame while the thread is in `set_loop`.
  For each `(a,b)` pair on the bound unit:
  - If the level `unit+0x2E` lies inside the unit's vertical span `[+0x0A − +0x2A, +0x0A]`
    (HYPOTHESIS: a water/ground surface crossing): `0x80078250(unit, a, b, 0, 160,
    300+rand&0x7F, 66|-1, …)` → node pos `0x800745D0` → `0x80086C9C`.
  - otherwise: `0x8007832C(unit, a, b)` (25% chance) → node pos → `0x80086854(0, pos, 0x8008E528)`.
- Both spawners take a slot from the shared `0xE0` pool (`0x80089ECC`, base `0x801AEB3C`).
  They install **FX thinks only**: `0x8008523C` (class 2) and `0x800853D4` (class 5).
  Neither think calls the object raycast `0x8006D534` or the router `0x800835E8`, and
  their only callee is `0x80056B54`.
- `(a,b)` are **model-node indices on the bound unit**, not world coordinates. So `op_f`
  can't place anything at an arbitrary location.
- `0x80086854`/`0x80086C9C` are the same spawners the projectile-hit code calls right
  after the router (`0x800838C4` → `0x800838D4`). They are impact/explosion sprites.
  This matches `DESTRUCTIBLE_OBJECTS.md` ("shrapnel burst").

**Still useful:** `op_f` + `set_loop n` is the data-only **visual** layer for a strike
(smoke/shrapnel at a bound unit's nodes for `n` frames).

## 2. The primitive to reuse — segment hitscan `0x8007A334`  CONFIRMED
`hitscan(owner a0, seg a1, desc a2)`: `seg` = 2 × (3×s16) `{start, end}`.
1. `hit = 0x8006D534(…, seg delta, &hitPos)` — the object raycast (entity AABBs + props).
2. If something was hit: `0x800835E8(owner, hit, desc, &delta, &hitPos)` (damage router),
   then FX `0x800873F8` (sparks) and `0x800869F0(…500, 6…)` (explosion).
3. `0x800705D8(seg, &out, mask 7)` — the world-geometry ray. If the hit section record has
   `+0x30 & 0x20000` (destructible), FX plus `0x800835E8(owner, sectionRec, desc…)`.
   So **world destructibles (doors, generators) take damage through the same call**.

Callers: the player blade `0x8007A4F4` (×2 half-segments), `0x8007AC40/50`, and every
mission unit's enemy blade (e.g. FDAT 16 `0x801C95A0/B0`). It needs no `0xE0` pool entry.

### Minimum viable "synthetic projectile" = a 4-byte damage descriptor  CONFIRMED (calc) / HYPOTHESIS (hooks)
`0x800835E8(owner, target, desc, dir, hitPos)` → for the entity range
`[0x801A26B8, +0x1590)`: `0x80075350(target, desc, dir, hitPos)`. The damage calc reads
only two things:
- `desc[0] & 0x30`: type (`0x10` shell / `0x20` energy).
- `*(u16*)(desc+2)`: attack.

| target | calc | formula |
|---|---|---|
| player (slot 0, by address) | `0x80075280` | `def = 0x10 ? *0x80041214 : 0x20 ? *0x80041215`; `((atk*def)>>6)*2/3`, then ×2 in `0x80075350` |
| NPC | `0x800752F4(tmpl+0x14, desc)` | energy: `atk*(8 − nibble)>>3 + 1`; **shell: raw `atk` (no defense)** |

HYPOTHESIS: per-mission `+0x5C` on-hit hooks only read `desc[0..3]`. Check one hook
(recipe D-2) before relying on a 4-byte struct. Otherwise copy a real weapon-def entry
(`0x8008E530`, stride `0x1C`).

## 3. Chosen mechanism  [NEEDS NEW CODE] — small
**Strike routine** (in the overlay-202 cave, see `PERCEPTION_MODEL.md` §4):
```c
void strike(s16 x, s16 y, s16 z, u16 radius, u16 atk) {
    static u8 desc[4] = { 0x10, 0, atk_lo, atk_hi };      // shell
    for (slot = 0; slot < 16; slot++) {                    // entities in radius
        e = 0x801A26B8 + slot*0x170;  if (!*e) continue;
        if (dx*dx + dz*dz > r*r || |dy| > h) continue;
        0x800835E8(/*owner*/0, e, desc, &dir, &e->pos);    // router (keeps +0x5C hooks, HUD hit marker logic)
    }
    // world destructibles + visuals: a vertical segment through the centre
    0x8007A334(0, {x, y+H, z, x, y−H, z}, desc);
    0x800869F0(0, &pos, 500, 6, …);                        // big explosion FX (args as in 0x8007A3F0)
}
```
**Trigger = a new chunk-4 verb through the tail mask (no dispatcher patch).**
Opcodes `0x000C`/`0x000D` already set bits `0x1000`/`0x2000`, and the tail never tests
them. Tested bits: `0x1, 0x4, 0x100, 0x200, 0x400, 0x800, 0x4000, 0x8000, 0x10000,
0x20000, 0x40000, 0x80000`.
Hook the tail with one or two words near `0x8008C004..0x8008C078`. Jump to a cave routine
that tests `s6 & 0x1000` and calls `strike(scratch s3 xyz, …)`, then re-executes the
displaced instructions. Risk: low. Scope: one overlay hook, ~60 instructions in the cave,
repack + checksum.

Authored strike (after the patch, **data-only per mission**):
```
sel_actor   k            ; bind a thread/unit
set_xyz     X, Y, Z      ; strike centre -> scratch s3
0x000C                   ; "strike" (sets tail bit 0x1000)
set_loop    1            ; 1 frame = 1 hit; n frames = n hits (napalm / DoT)
END
```
The trigger is timing within a set. "On a mission flag" means the set has to be launched
by primary cmd 2 (`0x8008A158` → `0x8008C2FC(set)`), and that's called from
objective-object **code**. Generic bridge: a cave watcher that polls a condition (door HP
sentinel `0xFFFF`, `0x801D0B50`, a flag bit) and calls `0x8008C2FC(set)` once. That's
about 10 instructions on the same hook.

## 4. Correction to the "unknown opcodes are no-ops" constraint  CONFIRMED (static)
Opcodes `< 0x20` with no handler still set their mask bit. Four of those bits drive real
tail callees:

| op | bit | tail action |
|---|---|---|
| `0x0010` | `0x10000` | `jal 0x8008792C` |
| `0x0011` | `0x20000` | `jal 0x800736AC` (call method `+0x14→+0` on all 512 × 52-B records at `0x80198B08`) |
| `0x0012` | `0x40000` | `jal 0x80073104` |
| `0x0013` | `0x80000` | `0x8019F524 |= 0x400`; `0x800796F4` (→ `0x8007959C` mission tick, `++0x801A5DC4`) |

They do nothing when there's no `set_loop`, because the tail only runs while state ≠ 0.
That's why the disc walk saw no effect. Opcodes `>= 0x20` that aren't in the table really
are pure no-ops.

## 5. "Breach the door → defenders" — two paths
1. **Extra damage at the breach:** same `strike` (§3). `0x8007A334` already routes world
   destructibles (section `+0x30 & 0x20000`). [NEEDS NEW CODE] (shared with §3)
2. **Defenders appear [DATA-ONLY]:** the proximity spawner `0x80078F00` (per spawn record,
   44 B at `0x8019FAB8`) spawns a record when its section `rec+0x12` enters the
   **player's active set** `0x801A5DD8` (`PERCEPTION_MODEL.md` §2).
   - `rec+0x14 & 1` adds box mode: it also triggers when inside a box (s8×256 extents at
     `+0x22..+0x27`) around object `rec+0x16` of the `0x80198B08` array.
   - `rec+0x14 & 0x40` = skip; `rec+0` = spawned flag; `rec+0x0A == −1` = absent.

   Author defenders in the room behind the door. They spawn when the breach puts that room
   into view. Edit with `tools/mission/spawn_edit.py` (chunk 12). Field semantics past the
   flags above are HYPOTHESIS. Alternative trigger: `spawn_group 0x1003 N` in a set
   launched by the §3 watcher.

## 6. Open / live recipes
- **D-1 — capture a real descriptor:** exec breakpoint `0x8007A334` while an enemy AC
  swings its blade (any mission with a blade AC). Read `a2` → 28 B; compare with
  `0x8008E530` entries.
- **D-2 — hook read-width:** exec breakpoint the struck NPC's `+0x5C` handler (read the word
  at `ent+0x5C`) when hit. Single-step it and note every `lbu/lhu/lw` off `a1` (desc).
  Max offset < 4 ⇒ the 4-byte descriptor is safe.
- **D-3 — RAM-only strike proof (no repack):** freeze the scene (`ENTITY_AI_FRAMEWORK.md`
  §2). Write a desc `{10 00 C8 00}` to scratch RAM. Patch the `+0x58` of a frozen NPC to a
  tiny RAM stub that calls `0x800835E8(0, slot2, desc, …)` once. HP `+0x160` should drop
  by 200.
- Spawn-record `+0x12/+0x14/+0x16/+0x22..27` semantics: confirm with one authored record
  (spawn on room entry).
