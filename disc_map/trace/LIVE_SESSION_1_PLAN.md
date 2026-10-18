# Live session 1 — plan (2026-10-18)

Goal: close checklist items **S-1**, **S-5**, **S-3** in one sitting and seed the save-state
library with its first entries. Driver: Byron on the pad. Trap-setter: Claude over the
DuckStation MCP server (`localhost:2346`). Checklist: `docs/LIVE_CHECKLIST.md`.

Rule for the whole session: **a slot is scratch until it is archived with an ID.** Nothing
in a doc may cite a slot number.

---

## 0. Before launching DuckStation

| # | Who | Do | Why |
|---|---|---|---|
| 0.1 | Byron | Install the zstd CLI (user scope, no admin): `winget install --id Facebook.zstd -e --scope user` — then open a new terminal and check `zstd --version`. | `tools/duckstation/savestate.py` shells out to `zstd` to decompress the state; it is not on this PC yet. Without it step 1.3 is skipped and the reader stays unvalidated. |
| 0.2 | Claude | Register the DuckStation MCP server for Claude Code (`.mcp.json`, streamable HTTP, `http://localhost:2346/mcp`). Needs a session restart to take effect; fallback is the inline Python client already used today. | Makes `breakpoint`, `read_memory`, `save_state` etc. first-class tools. |
| 0.3 | Byron | Launch DuckStation from the desktop shortcut. **Pristine disc**, not a test disc. Decline any update prompt. Pause menu → make sure nothing is in slots 1–5 (fresh install: nothing is). | The reader anchors on the pristine EXE; a test disc would also confuse the stage reads. |
| 0.4 | Claude | `get_status(detail=full)` → confirm serial `SLUS-01323`, record emulator build, frame counter. | Session header for the results log. |

Known emulator gotchas (from `DESTRUCTIBLE_OBJECTS.md`, `COMBAT_PHYSICS.md`):
- **Read-watchpoints do not fire.** Use execute breakpoints and write-watches only.
- Watch the **aligned word**: a byte/halfword watch on an odd address misses halfword/word stores.
- `input_sequence` state persists across `load_state`; we will not use it today — pad only.
- `search_memory` is unreliable; read a block with `read_memory` and parse it instead.

Slot plan for today:

| slot | content | archive as |
|---|---|---|
| 1 | garage, after the first boot | `GAR-main-01` |
| 2 | mission 1, NOW LOADING finished, before player control (if we can catch it) | `M01-load-01` (optional) |
| 3 | mission 1, first frame of player control | **`M01-ctrl-01`** ← S-1 |
| 4 | mission 1, standing in front of the HP test unit, before the first hit | `M01-mid-01` |
| 5 | scratch / retries | — |

---

## 1. Boot, garage state, reader validation  (S-1 prerequisite)

| # | Who | Do | Look for |
|---|---|---|---|
| 1.1 | Byron | Boot the game. Title → Start → Scenario Mode → New Game → name entry (any name) → garage. Stop in the garage main menu. | — |
| 1.2 | Claude | `pause()`, `save_state(slot=1)`, `continue()`. | File `SLUS-01323_1.sav` appears in `OneDrive\Documents\DuckStation\savestates`. |
| 1.3 | Claude | Run the offline reader on it: `python tools/duckstation/savestate.py <savestates>/SLUS-01323_1.sav --disc "$AC1_BIN" --read 0x8019F51C`. Also print the header's state-version word (`+0xa8`). | Expect `stage_byte=0`, `in_mission=False`, objobj ptr `0`. **If the state version is not 82** the header layout may have shifted on build 12000 — note it; the reader anchors RAM by content so it may still work. If `zstd` is missing the reader fails: record "reader unvalidated" and carry on. |
| 1.4 | Claude | Cross-check live: `read_memory(0x8004121B, 1)`, `read_memory(0x800411F8, 1)`, `read_memory(0x8019F51C, 4)`. | Must match the offline reads from 1.3 byte for byte. That is the validation. |

Result to log: reader OK / not OK, state version, `GAR-main-01` archived.

---

## 2. Mission 1 and the first in-mission state  (S-1)

The mission with the HP test unit is internal mission **1** (FDAT entries 2/3). Its spawn
table (`tools/mission/mission_parse.py --stream disc_map/extracted/FDAT_T/entries/003.bin --spawns`)
has 27 records; the one we want is:

| idx | type (hw7) | blk (hw3) | X | Y | Z | rot | hw8 | hw11 | hw12 | hw15 |
|---|---|---|---|---|---|---|---|---|---|---|
| **4** | 5 | 2 | -2552 | 0 | 73 | 1024 | 64 | **9800** | 300 | 513 |

It is the only record in the mission with a big distinctive HP and it sits at the stage
origin. Everything else is 2050 (blk 1, hw12=313) or 1500 (blk 3). Mission 0 is the fallback
test bed (two records with hw11=5200: idx 4 type 34, idx 5 type 25).

Runtime addresses derived from the table (template stride 44 = 0x2C):

| what | address | note |
|---|---|---|
| template record 4 | `0x8019FAB8 + 4*0x2C` = **`0x8019FB68`** | 44 bytes |
| record `+0x02` (seeded from hw11) | `0x8019FB6A` (hw) | **watch the aligned word `0x8019FB68`** |
| record `+0x1A` (= hw11 itself) | `0x8019FB82` (hw) | **watch the aligned word `0x8019FB80`** |
| entity array | `0x801A26B8 + k*0x170`, k = 0..15 | idx 0 = player; the unit's slot k is found live (2.5) |
| entity HP | `entity + 0x160` (u16) | player's is `0x801A2818`; HUD shows value ÷4? (`RUNTIME_RAM_MAP.md`: 4408 ⇔ 17632) |

| # | Who | Do | Look for |
|---|---|---|---|
| 2.1 | Byron | Garage → Mission → pick the mission that turns out to be internal #1 (we verify in 2.3; if it is not #1, note which it is and either continue with that mission's 9800/5200 unit or back out and try the next list entry). Sortie. | NOW LOADING. |
| 2.2 | Claude (optional) | The moment the loading screen clears and before Byron touches the pad: `pause()`, `save_state(slot=2)`, `continue()`. | `M01-load-01`. Skip if timing is awkward. |
| 2.3 | Byron | Say "I have control" on the first frame the AC responds. Do not move yet. | — |
| 2.4 | Claude | `pause()`. Read: `0x800411F8` (mission byte, expect `01`), `0x8004121B` (stage byte — S-4 side-data, expect `00`), `0x8019F51C` (objobj ptr, expect `0x801C4B40`), `0x8019F524` (flags), `0x80048610` (result). `get_status` → frame number. | objobj ptr non-zero ⇒ in-mission. |
| 2.5 | Claude | `read_memory(0x8019FB68, 44)` → confirm hw11 at `+0x1A` reads `0x2648` (9800) and `+0x02` reads the same. `read_memory(0x801A26B8, 0x1700)` → scan the 16 entity slots for `+0x160 == 9800` (`0x2648`) or `39200` (`0x9920`); also note each slot's `+0x0E` (dispatch type) and `+0x08/+0x0C` position; the unit should be at ~(-2552, ·, 73). Record **E** = the unit's entity address. | Pins the three HP candidates to concrete addresses. |
| 2.6 | Claude | `save_state(slot=3)`. **Archive immediately** as `M01-ctrl-01` (serves S-1, LVL-1, LVL-2; purpose "first frame of player control, mission 1, pristine disc"). | S-1 done. |
| 2.7 | Claude | While still paused: `dump_ram(path=…\M01-ctrl-01.ram.bin)` into the library folder next to the state. | Gives `overlay2c.sh` a fresh mission-overlay RAM image (replaces the stale Jun-6 dump mentioned in `tools/README.md`). |

Result to log: `M01-ctrl-01` archived, E, the three HP addresses, stage byte value.

---

## 3. HP ground truth  (S-5)

| # | Who | Do | Look for |
|---|---|---|---|
| 3.1 | Claude | `continue()`. | — |
| 3.2 | Byron | Walk to the test unit at the stage origin (-2552, 0, 73). Stop with it in the rifle's lock window. Do **not** fire. Say "ready". | Type 5 / block 2 is a large static structure; it should be visible from the start. |
| 3.3 | Claude | `pause()`, `save_state(slot=4)` → archive `M01-mid-01` (purpose "in front of HP test unit, pre-hit", parent `M01-ctrl-01`, serves S-5, ENT-9). Read and record the baseline: `0x8019FB68` (4 B), `0x8019FB80` (4 B), `E+0x160` (6 B: the three copies +0x160/162/164). | Baseline values. |
| 3.4 | Claude | `breakpoint(add, write, 0x8019FB68)`, `breakpoint(add, write, 0x8019FB80)`, `breakpoint(add, write, E+0x160 aligned down to 4)`. `continue()`. | Three write-watches armed. |
| 3.5 | Byron | Fire **one** rifle round at the unit. | Emulator pauses on the first watch hit. |
| 3.6 | Claude | On each pause: `read_registers()` → `pc`; `disassemble(pc-8, 6)`; re-read the three fields; `continue()` until no more hits for ~2 s of play (Byron holds still). Repeat the whole thing for a **second** round. | **Which field decremented, by how much, and from which `pc`.** Expected: `E+0x160` drops via the damage route (`ENTITY_AI_FRAMEWORK.md` §4, `hp -= dmg`). The open question is whether `+0x02` or `+0x1A` of the template mirrors it (or neither). The decrement = one rifle round's damage. Note whether the number is ×4 of the HUD/display scale. |
| 3.7 | Claude | `breakpoint(clear)`. Write the result line for S-5. | Settles `OBJECT_STATS.md` §1/§4 (hw11 = HP), the `DESTRUCTIBLE_OBJECTS.md` record discrepancy (`+0x02=+0x1A` vs `+0x00=+0x18`), `AUTHORING.md` Phase 1 `[~]`. |

Optional 3.8 (only if slot 2 exists): `load_state(slot=2)`, `breakpoint(add, execute, 0x80078B14)`, `continue()`; on each hit log `a0`/`a1` and count hits until the mission is live → how record index `i` maps to the entity slot `k`. Then `breakpoint(clear)`.

---

## 4. The `0x8004F024` dispute  (S-3)

Two docs disagree: `COMBAT_PHYSICS.md` §6 says `0x8004F024` is the AC movement integrator
(velocity at `ac+0x64` → position `ac+0x08/+0x0C`, via `0x8004EF74`), called from the enemy
update wrapper `0x800760DC`. `COLLISION.md` reads it statically as a timed value-easing routine
with no world reads. One of them is mis-scoped.

Mission 1 has no enemy AC, but the entity array is shared by every moving entity, and the
wrapper is the enemy per-frame update, so any **moving enemy** (MT) qualifies. If nothing moves
near the start, the player's own update path is the second probe (4.5).

| # | Who | Do | Look for |
|---|---|---|---|
| 4.1 | Claude | `load_state(slot=3)` (= `M01-ctrl-01`). `breakpoint(add, execute, 0x8004F024)`. `continue()`. | — |
| 4.2 | Byron | Move toward the nearest enemy until something is clearly moving on screen. | Pause on the breakpoint. |
| 4.3 | Claude | On hit: `read_registers()` → `a0` (= entity? check it is inside `0x801A26B8..+16*0x170`; note `k`), `ra` (caller — is it inside `0x800760DC`?). Read `a0+0x08`, `a0+0x0C`, `a0+0x10`, and `a0+0x60..+0x80` (the "velocity/orientation around +0x64" region). | Identity of the argument and the caller. |
| 4.4 | Claude | `breakpoint(add, write, a0+0x08)` and `breakpoint(add, write, a0+0x0C)`. `step_out()` → `wait_for_pause()`. | **If a write-watch fires with `pc` inside `0x8004EF74..0x8004F100` and the new `+0x08` equals old `+0x08` plus something read from `+0x64..`: integrator (COMBAT reading wins).** If `0x8004F024` returns with `+0x08/+0x0C` untouched and only some other field eased toward a target over frames: value-easing (COLLISION reading wins; then find who *does* write `+0x08` — leave the write-watch armed and `continue()` once). |
| 4.5 | Claude | If `a0` was never the player (`0x801A26B8`): note that the player path differs (`+0x4C = 0x80079888`). Optionally arm the `+0x08` write-watch on the player and have Byron walk, to log the player's integrator `pc` for `CMB-1`. | Side-data for CMB-1. |
| 4.6 | Claude | `breakpoint(clear)`. Write the S-3 result. | Fix whichever doc is wrong; update `ENT-1`/`CMB-1` context in the checklist. |

---

## 5. Close-out (Claude, with Byron confirming)

1. Archive anything still only in a slot. Slots 1–5 are then disposable.
2. `LIVE_CHECKLIST.md`: tick S-1, S-5, S-3 with date + one-line result; append the **Results log**
   line: date · items closed · state IDs · docs updated.
3. Fold results into the owning docs: `SAVESTATE_FORMAT.md` (in-mission state now exists; state
   version; VRAM offset still open → LVL-1 is now unblocked), `OBJECT_STATS.md` §1/§4,
   `DESTRUCTIBLE_OBJECTS.md` record layout, `COMBAT_PHYSICS.md` §6 or `COLLISION.md` "Disputed",
   `AUTHORING.md` Phase 1, `tools/README.md` RAM-dump note.
4. Queue for session 2: **S-4** (stage byte in three missions + arena + garage, with a write-watch
   from the menu), **S-2** (boot the capstone disc — Claude builds it offline before the session),
   **LVL-1** (VRAM offset, now possible from `M01-ctrl-01`).

## Results (2026-10-18)

The session ran out of order (states were saved before the plan was read) and in **mission 30
"Eliminate Squatters"** instead of mission 1. Outcome: S-1, S-3, S-5 closed — see the
`LIVE_CHECKLIST.md` rows and Results log for the numbers. States: `GAR-main-01`,
`M30-ctrl-01` (+ RAM dump), `M30-mid-01`. Lessons: (1) convert addresses in hex, never by
hand — the first watch set landed 0x80000 high; (2) checkpoint a slot before every resume,
the mission keeps running (incoming fire, timer); (3) the DuckStation state-version word
is at header `+0x04` (87 on build 0.1-12000), not `+0xa8`.

Abort / retry notes: NOW-LOADING hang = checksum (only on a test disc, not today). If a watch
never fires, confirm it is on the aligned word. If the pad stops responding after a `load_state`,
`pause()`/`continue()` once.
