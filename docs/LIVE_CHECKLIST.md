# Live-confirmation checklist (DuckStation)

Every open item in the RE that is **blocked on the running game** — collected in one
place so an emulator session can be mechanical: load a state, set the trap, read the
answer, tick the box. Each item names the address/trap, what to look for, and **which
doc claim it settles**, so results can be folded straight back into the right file.

Conventions:
- `[ ]` open · `[x]` done (put the date + one-line result in the **Result** column and
  update the owning doc) · `[-]` dropped (say why).
- Traps: `bp` = execute breakpoint · `ww` = write-watch · `rw` = read-watch. Watch the
  **aligned word** — a byte-watch on an odd address silently misses halfword/word stores
  (`INTERACTIONS.md` §1). For a flag that a driver rewrites every frame, NOP the driver's
  store, catch the real setter, restore the bytes (`MISSION_SYSTEM.md` §6 — the
  `0x8019F524` masker is the `sw` at `0x8008A938`, bytes `24 f5 22 ac`).
- Method for "what changed": idle-baseline diff of `0x801B0000–0x801E0000` (0 idle churn),
  then a watchpoint on the hit (`INTERACTIONS.md` §5). Render/OT lists, the SsVm sound
  scratch and the stack are red herrings — list in `INTERACTIONS.md` §4.
- All overlay addresses are FDAT entry **202** at `0x8004ADA0` unless marked *objobj*
  (per-mission objective object at `0x801C4B40`, FDAT `2N`) or *EXE* (resident).
- **Adding items:** append to the right table with the next ID; one row = one trap +
  one answer. Keep the "Settles" column pointing at a doc + section.

Priority order: the first block (**S**) is the short session that unblocks the most;
the rest can be done in any order.

---

## S. Short session — do these first (each unblocks several items)

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| S-1 | [ ] | Save a DuckStation state **inside any mission** (after the PA loader has run — e.g. first frame of player control). Keep it with the other backups. | A `.sav` whose `stage_byte()`/`in_mission()` is true in `tools/duckstation/savestate.py`. | `SAVESTATE_FORMAT.md` "Findings" (no in-mission state exists); prerequisite for LVL-1/LVL-2 and every offline VRAM question. |
| S-2 | [ ] | Boot the offline **capstone disc** (`…[CAPSTONE].bin`, `AUTHORING.md` capstone) and play mission 0. | Edited geometry visible; edited spawn at its new position; edited HP (takes the predicted number of hits); edited script behaviour. Any NOW-LOADING hang = checksum. | `AUTHORING.md` "Status vs. plan": Phase 0 + capstone "in-game confirmation pending". Also doubles as MIS-1's HP test bed. |
| S-3 | [ ] | `bp 0x8004F024` while an enemy AC is moving. Inspect `a0` (= ac), then step: does the body read `ac+0x64` and write `ac+0x08/+0x0C`? | Writes to `ac+0x08` from `ac+0x64` ⇒ integrator (COMBAT reading). No world/position writes, just a timed value ease ⇒ COLLISION reading. | The `0x8004F024` dispute: `COMBAT_PHYSICS.md` §6 vs `COLLISION.md` "Disputed". Fix whichever doc is wrong. |
| S-4 | [ ] | Read `0x8004121B` (and `0x8004121D..0x1224`) at `bp 0x8004F1A8` in **three different missions**, plus once in the arena (overlay 204) and once in the garage/mission-assignment screen. `ww 0x8004121B` from the menu through SORTIE. | Byte value per context; the `pc`/`ra` of any write. | `MISSION_STAGES.md` Part 1 vs `LEVEL_LOADING.md` (is the byte ever non-zero? who selects PA01–PA71?). |
| S-5 | [ ] | Mission 1: `bp 0x80078B14`; note the instance template `0x8019FAB8 + i*44` for the `hw11=9800` unit. Deal one hit. `ww` the aligned words at template `+0x02`, `+0x1A` and entity `+0x160`. | Which halfword decrements, by how much. | `OBJECT_STATS.md` §4 (hw11 = HP, "ground-truth needed"); `DESTRUCTIBLE_OBJECTS.md` "Record layout" discrepancy (`+0x02=+0x1A` vs `+0x00=+0x18`); `AUTHORING.md` Phase 1 `[~]`. |

---

## MIS. Mission state / objectives / script VMs

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| MIS-1 | [ ] | Same setup as S-5; after HP is pinned, on the capstone disc edit `hw8`, `hw9`, `hw12`, `hw15` one at a time on one spawn record (`tools/mission/spawn_edit.py`) and observe the unit. | Behaviour change per field (range? AI flags? area? orientation?). `hw9` is also the flag-table row (`0x80198B08`, row stride `0x6c`) — a `rw` there confirms. | `MISSION_SYSTEM.md` §2 table (`hw8/9/12/15` HYPOTHESIS); `OBJECT_STATS.md`. |
| MIS-2 | [ ] | Mission with a timer (cmd 4): `bp 0x8008A178`, read `a0`/param, then read `0x8019F52C` right after and the HUD MM:SS. Frame-step 60 frames. | Initial value vs `22×param`; decrement per frame (1/frame?); the unit (frames@60 or 50). | `MISSION_SYSTEM.md` §3 "×22 / unit HYPOTHESIS". |
| MIS-3 | [ ] | `bp 0x8008A260` (cmd 8) and `bp 0x8008A274` (the commit). Step through `FUN_80052A2C(99)`. | What prompt/message id 99 shows; what input makes it return 1; whether any disc mission reaches cmd 8. | `MISSION_SYSTEM.md` §4 "prompt-gated success — exact trigger should be confirmed live". |
| MIS-4 | [ ] | A mission with ≥2 objectives: `ww 0x801D0B50` and `ww 0x8009079C` together; destroy objectives one by one. | Do both move? Does `0x801D0B50` count destructibles (DESTRUCTIBLE-COUNT) while `0x8009079C` is the intro/phase step? | `MISSION_SYSTEM.md` §6 "reconcile `0x801D0B50` with `0x8009079C`"; `ENTITY_TYPES.md` taxonomy rows DESTRUCTIBLE-COUNT / PER-PHASE STEP. |
| MIS-5 | [ ] | NOP the masker `0x8008A938`, `ww 0x8019F524`, play until the exit. | `pc` of the write that sets bit `0x400`. Candidates: the secondary-VM action bit `0x80000` (`MISSION_SCRIPT_VM.md` tail, `0x8008BC04–0x8008C078`) and the destructible action bit `0x80000` (`DESTRUCTIBLE_OBJECTS.md`). | `MISSION_SYSTEM.md` §5/§6 "`0x400` setter not yet pinned". |
| MIS-6 | [ ] | `bp 0x8008B830` and `bp 0x8008B42C`; each hit dump the thread table `0x801D0B60` (24 B/thread, count `0x801D0B51`, set idx `0x801D0B50`). On the capstone disc vary `init_unit` hw1–5 and watch `unit+0x65/74/76/78/7A/7C/15C`. | Live thread execution order vs the disassembler's; which `init_unit` operand lands in which unit field and what it does (velocity? id? HP?). | `MISSION_SCRIPT_VM.md` "Open" (thread order, `init_unit`/`mark_lerp`/`op_f` fields); `AI_PATHS.md` (`init_unit` hw1–5). |
| MIS-7 | [ ] | `ww 0x801D0B64` from mission load. | `pc`/`ra` of the writer = the chunk-4 stager. | `MISSION_SCRIPT_VM.md` "Writer of `0x801D0B64`". |
| MIS-8 | [ ] | Mission 2: run twice — (a) destroy placement records 39–42, (b) keep them alive until the exit. | Which run commits (`0x8019F524`, `0x80048610`). | `ENTITY_TYPES.md` "M2 protect-vs-destroy sign". |
| MIS-9 | [ ] | Missions 20, 24, 28: `bp 0x80078CFC` (spawn entry) from load; log `ra` and the template index per hit. | Whether entities arrive via secondary-VM `spawn_group`, the objobj EVENT method, or a transition from a previous mission. | `ENTITY_TYPES.md` "Missions 20/24/28 (no chunk-12)"; `LEVEL_LOADING.md` §7 shared-scene selection. |
| MIS-10 | [ ] | `rw 0x801FD844`; then `ww` on `*0x801FD844+0x10`. | Which subsystem owns the READY-GATE word (text/prompt VM? sound? fade?). | `ENTITY_TYPES.md` "`0x801FD844` subsystem identity"; `MISSION_SYSTEM.md` §4 correction (ready gate `FUN_80052338`). |
| MIS-11 | [ ] | Follow-up to S-4. If the byte is ever non-zero: `bp 0x8004F1A8` in that context and trace the caller chain back to the menu / sortie code (*EXE*). If never: `bp 0x8004F1A8` outside missions (assignment screen, arena, garage) and record which PA file each context opens. | The real selector for PA01–PA71. | `MISSION_STAGES.md` Part 1 "where the stage byte VALUE is set — OPEN"; closes the MISSION_STAGES/LEVEL_LOADING reconciliation. |
| MIS-12 | [ ] | `bp 0x80078D50` (type dispatch) in every mission you visit; log `(mission, type=instance+0x0E, fn)`. | The concrete type→think map, to merge into `disc_map/type_catalog_long.csv`. | `ENTITY_TYPES.md` "Concrete numeric type→think for all 59 missions". |
| MIS-13 | [ ] | `bp` each AUX_TBL `+0x120` entry of the objobj (M0–M3 first); note the caller and the `0x8019F524`/flag bit it touches. | Per-mission event-flag bit meanings. | `ENTITY_TYPES.md` "AUX_TBL `+0x120` bit semantics"; `0x80198808` row format. |
| MIS-14 | [ ] | `bp` the primary-VM jump-table cases 00/01/02/09 (`0x8004C164`; bodies `0x8008A10C…`). | What each actually does when hit in a real mission. | `overlays/OVERLAY_MAP.md` "hyp meaning" cases. |
| MIS-15 | [ ] | With `0x8019F524` holding `0x400` (after MIS-5, or poke it), `bp 0x8004C318` and step the store. | Does the word become `a0` (store) or `a0 \| old` (OR)? | `MISSION_SYSTEM.md` §4 "writes a0 directly" vs the live `0x480` at exit; `re/ac1_mission` `set_result` semantics; authoring of `set_result` scripts. |
| MIS-16 | [ ] | Complete a mission via the exit border (cmd 5 path): read `0x80048610` the frame the fade ends, then `rw 0x80048610` into the debrief (*EXE*). | The result code for a `0x80` exit and how the base EXE decides success vs fail from it (or from `0x8019F524` directly). | `MISSION_STATE_MACHINE.md` §6; `MISSION_SYSTEM.md` §4 result-code table (0x80 → 0?). |

---

## ENT. Entities / AI / collision / RAM map

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| ENT-1 | [ ] | On an **enemy AC**: zero `+0x58` only; `ww` its `+0x08/+0x0C` and `+0x20/+0x24`. Repeat on the **train** (slot 7 in the train mission) and an **aircraft**. | Per class: which slot drives movement, which drives fire, and which position fields are authoritative (`+0x08/+0x0C` int32 vs `+0x20/+0x24`). | `COMBAT_PHYSICS.md` §6 (`+0x58` = fire AI) vs `ENTITY_AI_FRAMEWORK.md` §1–2 (`+0x58` = think; nulling corrupts pos) — slot-role note; `RUNTIME_RAM_MAP.md` "Position offset to reconcile". |
| ENT-2 | [ ] | = S-3. | | `0x8004F024` dispute. |
| ENT-3 | [ ] | Capstone disc: add `spawn_group` opcodes until `0x80078A2C` (allocator) returns no slot; read the head words of `0x801A26B8 + k*0x170`. | The real slot count (15? 16? 24?). | `ENTITY_AI_FRAMEWORK.md` "15-vs-16 slot count"; `COLLISION.md`; `RAMWATCH_CONCORDANCE.md` §D. |
| ENT-4 | [ ] | In-mission, read `0x801A25E8` region (the `0xD0`-stride pool) and compare with the `0x170` array. | Whether the `0xD0` pool is live at all during a mission or only in the garage loop. | `ENTITY_AI_FRAMEWORK.md` / `RUNTIME_RAM_MAP.md` "how the `0x170` array relates to the `0xD0` pool". |
| ENT-5 | [ ] | One session on the player entity: rotate (read `+0x12`), jump (`+0x14`), note `+0x6A/+0x6C`; read camera `0x801AD66C` + matrix `0x801AD67C` while panning; read `0x80198804` / `0x801AC81C` across frame-steps. | Confirm/deny each HYPOTHESIS row. | `RUNTIME_RAM_MAP.md` per-entity offsets, camera, frame counters; `COMBAT_PHYSICS.md` §1 player fields. |
| ENT-6 | [ ] | Mission 8: `bp 0x8008BC8C` (lerp tail) and `bp 0x801CAD44` (*objobj* route commit); `ww entity+0x08` and `entity+0x10C`. | Lerp direction (toward track1 or from it); train step per segment; any `copy_pos_hi` use. | `AI_PATHS.md` "Open" (lerp direction, train stepping, route-segment fields `+0x12..0x1A`, flags `+0x02`). |
| ENT-7 | [ ] | Aircraft mission: `bp 0x801C66DC` / `0x801C68E4` (*objobj* thinks); step the state byte `+0x40`. | The pattern each think implements. | `AI_PATHS.md` "aircraft patterns not yet individually decoded". |
| ENT-8 | [ ] | Walk into a wall: `bp 0x8006EE8C`; inspect the resolved contact; then `ww` the occupancy bit-grid region to find its builder; dump one 20-byte face. | Contact resolution; grid builder site; whether the face carries a plane. | `COLLISION.md` "Open" items. |
| ENT-9 | [ ] | After S-5: `bp 0x801C58C4` (*objobj* on-hit) on a generator; read the hit record. | Which field is current HP for **world-structure** objects (`0x801D0B68` array) vs MTs. | `DESTRUCTIBLE_OBJECTS.md` discrepancy; HP of other generators; pulse-rifle damage value. |

---

## CMB. Combat / physics

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| CMB-1 | [ ] | Arena: `ww 0x801A26C4` (player Z), then step into the writer's body. | `pos += vel`, `vel += accel`, the boost (energy) and gravity terms, and the store sites. | `COMBAT_PHYSICS.md` §3 "Physics integrator (anchored, not fully decoded)". |
| CMB-2 | [ ] | Fire a rifle round: `bp 0x80083A10` (round think) / `0x80085510`. | Integrate/collide/explode structure; which fields are velocity and lifetime. | `COMBAT_PHYSICS.md` §5 / `DESTRUCTIBLE_OBJECTS.md` function reference. |
| CMB-3 | [ ] | Fire a missile: `bp 0x80056B54` and `0x8005C904`; read the turn table `0x80039C7C[type<<3]`. | Exact turn rate per missile type. | `COMBAT_PHYSICS.md` "Open" #3. |
| CMB-4 | [ ] | Change one part (head, then core, …) in the garage, sortie, read `0x80041214/15` and the four `0x800B9xxx` tables. | Which table is head/core/arms/legs; the float curve constant. | `COMBAT_PHYSICS.md` §8a "OPEN (minor)". |
| CMB-5 | [ ] | Hit the player with a shell weapon, then an energy weapon; `bp 0x80075350`. | Which damage-type bit is shell vs energy. | `COMBAT_PHYSICS.md` §8 "TBD". |
| CMB-6 | [ ] | NOP the `sb v,0x41(s1)` transition writes in `0x801C75BC`; set `ac+0x41` to 0, 8, 9. | Behaviour of each state (idle / evade / retreat?). | `COMBAT_PHYSICS.md` "Open" #1. |
| CMB-7 | [ ] | `bp FUN_80085764`; fire each weapon class. | Weapon-def (`0x8008E530`, stride `0x1C`) → think map; the HYPOTHESIS field columns. | `COMBAT_PHYSICS.md` §4 weapon definition table; "Open" #4. |

---

## LVL. Levels / PA packs / textures

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| LVL-1 | [ ] | With the S-1 state: `tools/duckstation/savestate.py STATE --dump-vram` and compare against DuckStation's own VRAM dump of the same frame. | The exact GPU-component offset inside the v82 blob. | `SAVESTATE_FORMAT.md` "Caveat — VRAM offset". |
| LVL-2 | [ ] | From that VRAM: read pages `(704,256)` and `(768,256)` + the CLUTs the PA00 material words reference. | Real stage texels; whether they are 4/8 bpp; which CLUT rows are used. | `PA_FORMAT.md` texture section (texel source open); `AC1MOD_INTEGRATION.md` §1b. |
| LVL-3 | [ ] | `bp` the PsyQ `LoadImage` (libgpu, *EXE* — find via the PSYQ signatures) and/or `ww` the GPU DMA registers during mission load; log `(x,y,w,h)` + source RAM for every upload. | Which container/entry feeds the stage pages. Also confirms `0x8004F1A8` does **no** upload itself. | `PA_FORMAT.md` "RECONCILIATION" (texture bank NOT located); `PA_HEADER.md` ptr A/C/D payload hypotheses. |
| LVL-4 | [ ] | After the chunk-7 handler (`bp 0x80071E14` exit): read the lighting table and the per-section `+0x28` light index; vary the camera and read the orientation variant `k`. | Which on-disc chunk feeds the lighting records; the static source of `k`. | `LEVEL_LOADING.md` §7; `disc_map/trace/PLACEMENT_SOLVED.md` lighting note. |
| LVL-5 | [ ] | `rw` chunk 1's relocated region and the chunk-7 post-table tail during play. | What reads them (PVS/portal walk?). | `LEVEL_LOADING.md` §7 "chunk 1 / chunk-7 tail". |
| LVL-6 | [ ] | Load e200-using missions: `bp FUN_8004F508` and log every `read_T_entry` (`bp 0x800165E4`). | How a mission selects a shared scene; e200's header. | `LEVEL_LOADING.md` §7 shared-scene mechanism + e200 header. |

---

## INT. Interactions / COM / save

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| INT-1 | [ ] | `ww 0x80150000` (first dialog-text word) and open a door / pick up an item. | `pc`/`ra` of the COM-dialog writer → the queue structure. | `INTERACTIONS.md` §3 "queue not yet RE'd". |
| INT-2 | [ ] | Dump the memory card before and after a mission in which a weapon is picked up (and once without). | Where the obtained part lands in the main `…A` save. | `INTERACTIONS.md` §2 storage HYPOTHESIS; starts the main-save format. |

---

## Results log

Append one line per session: date · items closed · state file(s) saved · docs updated.

- *(none yet)*
