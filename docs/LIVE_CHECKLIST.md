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
| S-1 | [x] 2026-10-18 — `M30-ctrl-01` (mission 30 "Eliminate Squatters", first seconds of control, pristine disc) + `GAR-main-01`, `M30-mid-01` in the state library (`C:\Users\bross\dev\ac1_states`, manifest `disc_map/states/STATES.tsv`). State version word is **87** (header `+0x04`; the old "82" was read from `+0xa8`, which is the media-path length). RAM dump `M30-ctrl-01__20261017.ram.bin` alongside. Offline reader still unvalidated (no `zstd` on the PC yet). | Save a DuckStation state **inside any mission** (after the PA loader has run — e.g. first frame of player control). Archive it: `tools/duckstation/states.py archive <slot> M<NN>-ctrl-01 --serves S-1,LVL-1` and cite the ID (`disc_map/states/README.md`). | A `.sav` whose `stage_byte()`/`in_mission()` is true in `tools/duckstation/savestate.py`. | `SAVESTATE_FORMAT.md` "Findings" (no in-mission state exists); prerequisite for LVL-1/LVL-2 and every offline VRAM question. |
| S-2 | [ ] | Boot the offline **capstone disc** (`…[CAPSTONE].bin`, `AUTHORING.md` capstone) and play mission 0. | Edited geometry visible; edited spawn at its new position; edited HP (takes the predicted number of hits); edited script behaviour. Any NOW-LOADING hang = checksum. | `AUTHORING.md` "Status vs. plan": Phase 0 + capstone "in-game confirmation pending". Also doubles as MIS-1's HP test bed. |
| S-3 | [x] 2026-10-18 — **COMBAT reading wins.** Hit on MT entity slot 1 with `a0 = ent+0x64` (movement block), `a1 = ent+0x08` (position), `a2 = 0x46`, `ra = 0x801C5500` (*objobj* MT update). Body: `0x8004EDAC` → `0x8004EC4C` → `0x8004EFB8`, which per axis calls `0x8004EF74(blk+0x20/+0x28/+0x30, pos[i], blk.byte3)` and stores the result as **s16 at `+0x08/+0x0A/+0x0C` (X/Y/Z)**. `0x8004EF74` = Bresenham velocity: `counter(+4) -= step(+6)`; while `>0` return `pos + velA(+0)`, else reload `counter += byte3` and return `pos + velB(+2)`. Observed per frame: X −3, Y 0, Z +10 = the `velA` words (`fd ff / 00 00 / 0a 00`). COLLISION's "timed value-easing" was the counter, not the position. Player path (`0x800674B0`, CMB-1) not re-trapped. | `bp 0x8004F024` while an enemy AC is moving. Inspect `a0` (= ac), then step: does the body read `ac+0x64` and write `ac+0x08/+0x0C`? | Writes to `ac+0x08` from `ac+0x64` ⇒ integrator (COMBAT reading). No world/position writes, just a timed value ease ⇒ COLLISION reading. | The `0x8004F024` dispute: `COMBAT_PHYSICS.md` §6 vs `COLLISION.md` "Disputed". Fix whichever doc is wrong. |
| S-4 | [ ] partial 2026-10-21 — **0 in every context so far.** At `bp 0x8004F1A8` (PA loader, `ra 0x8004FA08`) in missions **20, 24, 28** (`M20/M24/M28-ctrl-01`): `0x8004121A..24` = `00 00 02 00 03 05 01 00 01 00 18` each time, so `0x8004121B` = 0. `ww 0x80041218` from the garage menu through SORTIE (3 runs): no write to `0x8004121B`; only the neighbour byte `0x80041218` ← 3 by `sb v0,0x24(fp)` @`0x8009D85C` (`ra 0x8009D66C`, front end). Post-load reads also 0 in the garage (`GAR-main-01`), mission 8, mission 34 (`M34-exit-01`) and mission 30 (S-1). Left: the arena (overlay 204) | Read `0x8004121B` (and `0x8004121D..0x1224`) at `bp 0x8004F1A8` in **three different missions**, plus once in the arena (overlay 204) and once in the garage/mission-assignment screen. `ww 0x8004121B` from the menu through SORTIE. | Byte value per context; the `pc`/`ra` of any write. | `MISSION_STAGES.md` Part 1 vs `LEVEL_LOADING.md` (is the byte ever non-zero? who selects PA01–PA71?). |
| S-5 | [x] 2026-10-18 — **HP = entity `+0x160`, seeded from spawn hw11.** Mission 30, MT slot 1 (`0x801A2828`, template rec 4 @`0x8019FB68`, hw7=41, hw8=0, hw11=2050): two rifle rounds → `+0x160` 2050→1832→1614, written by `sh v0,0x160(s0)` @`0x800753C8` inside `FUN_80075350` (dmg from `0x800752F4(tmpl+0x14 = hw8, hit)` = **218** = WG-RF35 attack power, no reduction at hw8=0). `+0x162` stays 2050 (max), `+0x164` = 8200 untouched (other MT: 3180 / 50880 — not ×4). Template `+0x02`/`+0x1A` **never written** (0 hits over 2 hits + seconds of play) ⇒ not live HP mirrors. Entity `+0x00` = ptr to its template record; `+0x0E` = hw3. | Mission 1: `bp 0x80078B14`; note the instance template `0x8019FAB8 + i*44` for the `hw11=9800` unit. Deal one hit. `ww` the aligned words at template `+0x02`, `+0x1A` and entity `+0x160`. | Which halfword decrements, by how much. | `OBJECT_STATS.md` §4 (hw11 = HP, "ground-truth needed"); `DESTRUCTIBLE_OBJECTS.md` "Record layout" discrepancy (`+0x02=+0x1A` vs `+0x00=+0x18`); `AUTHORING.md` Phase 1 `[~]`. |

---

## MIS. Mission state / objectives / script VMs

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| MIS-1 | [ ] | Same setup as S-5; after HP is pinned, on the capstone disc edit `hw8`, `hw9`, `hw12`, `hw15` one at a time on one spawn record (`tools/mission/spawn_edit.py`) and observe the unit. | Behaviour change per field (range? AI flags? area? orientation?). `hw9` is also the flag-table row (`0x80198B08`, row stride `0x6c`) — a `rw` there confirms. | `MISSION_SYSTEM.md` §2 table (`hw8/9/12/15` HYPOTHESIS); `OBJECT_STATS.md`. |
| MIS-2 | [ ] (setup found 2026-10-21: mission 28 shows an on-screen timer — start from `M28-ctrl-01`; the matched C already gives timer = param × 22, so only the tick unit is left) | Mission with a timer (cmd 4): `bp 0x8008A178`, read `a0`/param, then read `0x8019F52C` right after and the HUD MM:SS. Frame-step 60 frames. | Initial value vs `22×param`; decrement per frame (1/frame?); the unit (frames@60 or 50). | `MISSION_SYSTEM.md` §3 "×22 / unit HYPOTHESIS". |
| MIS-3 | [ ] | `bp 0x8008A260` (cmd 8) and `bp 0x8008A274` (the commit). Step through `FUN_80052A2C(99)`. | What prompt/message id 99 shows; what input makes it return 1; whether any disc mission reaches cmd 8. | `MISSION_SYSTEM.md` §4 "prompt-gated success — exact trigger should be confirmed live". |
| MIS-4 | [ ] partial 2026-10-21 — **REFUTED as a world-structure tally.** Mission 28 (a destruction mission whose hidden devastation score shows only on success): two world structures destroyed (records 108 and 112), **0 writes** to `0x801D0B50` (stays `0xFF`) or `0x8009079C` (stays `0x25`) over the whole fight. The devastation tally lives elsewhere — next: idle-baseline diff before/after destroying one car in `M28-ctrl-01` | A mission with ≥2 objectives: `ww 0x801D0B50` and `ww 0x8009079C` together; destroy objectives one by one. | Do both move? Does `0x801D0B50` count destructibles (DESTRUCTIBLE-COUNT) while `0x8009079C` is the intro/phase step? | `MISSION_SYSTEM.md` §6 "reconcile `0x801D0B50` with `0x8009079C`"; `ENTITY_TYPES.md` taxonomy rows DESTRUCTIBLE-COUNT / PER-PHASE STEP. |
| MIS-5 | [ ] | NOP the masker `0x8008A938`, `ww 0x8019F524`, play until the exit. | `pc` of the write that sets bit `0x400`. Candidates: the secondary-VM action bit `0x80000` (`MISSION_SCRIPT_VM.md` tail, `0x8008BC04–0x8008C078`) and the destructible action bit `0x80000` (`DESTRUCTIBLE_OBJECTS.md`). | `MISSION_SYSTEM.md` §5/§6 "`0x400` setter not yet pinned". |
| MIS-6 | [ ] | `bp 0x8008B830` and `bp 0x8008B42C`; each hit dump the thread table `0x801D0B60` (24 B/thread, count `0x801D0B51`, set idx `0x801D0B50`). On the capstone disc vary `init_unit` hw1–5 and watch `unit+0x65/74/76/78/7A/7C/15C`. | Live thread execution order vs the disassembler's; which `init_unit` operand lands in which unit field and what it does (velocity? id? HP?). | `MISSION_SCRIPT_VM.md` "Open" (thread order, `init_unit`/`mark_lerp`/`op_f` fields); `AI_PATHS.md` (`init_unit` hw1–5). |
| MIS-7 | [x] 2026-10-21 — **writer = `sw v0,0xb64(at)` @`0x8004F818`** (`ra 0x8004F800`, `s0 = 0x8019F518`), one write per load, in missions 20/24/28 (values `0x8011A8E4`, `0x800E36C4`, `0x801048D4` = `s1+4`; `M20/M24/M28-ctrl-01`). It runs right after the mission loader reads FDAT `2N`/`2N+1` (`read_T_entry` calls with `ra 0x8004F78C`/`0x8004F7AC`), i.e. ovl202's mission loader, not the base EXE | `ww 0x801D0B64` from mission load. | `pc`/`ra` of the writer = the chunk-4 stager. | `MISSION_SCRIPT_VM.md` "Writer of `0x801D0B64`". |
| MIS-8 | [ ] | Mission 2: run twice — (a) destroy placement records 39–42, (b) keep them alive until the exit. | Which run commits (`0x8019F524`, `0x80048610`). | `ENTITY_TYPES.md` "M2 protect-vs-destroy sign". |
| MIS-9 | [x] 2026-10-21 — **two different routes, neither is `spawn_group`.** **M20 and M24: section-visibility streaming spawner `0x80078F00`** (byte-matched) → `0x80078CFC` from `ra 0x8007908C`. M20: nothing at load; ~80 s in, templates 5 (type 1, section 16, HP 7900) and 14 (type 3, section 104, HP 2000). M24: template 4 (type 1, section 60 = the player's start section, HP 6500, flags `0x420`) at the end of the load. All had flags `+0x14` bit 0 and `0x40` clear. **M28: objobj code** — templates 6/7/8 (type 1, HP 1750, flags `0xD0`) from `ra 0x801C7B20`, three waves each (respawned after kills), and template 12 (type 2, HP 13800, flags `0x80C0`) from `ra 0x801C7A3C`. All M28 templates have bit `0x40` set — the matched spawner's "never auto-spawn" bit — so the mission code spawns them itself (`M20/M24/M28-ctrl-01`) | Missions 20, 24, 28: `bp 0x80078CFC` (spawn entry) from load; log `ra` and the template index per hit. | Whether entities arrive via secondary-VM `spawn_group`, the objobj EVENT method, or a transition from a previous mission. | `ENTITY_TYPES.md` "Missions 20/24/28 (no chunk-12)"; `LEVEL_LOADING.md` §7 shared-scene selection. |
| MIS-10 | [ ] | `rw 0x801FD844`; then `ww` on `*0x801FD844+0x10`. | Which subsystem owns the READY-GATE word (text/prompt VM? sound? fade?). | `ENTITY_TYPES.md` "`0x801FD844` subsystem identity"; `MISSION_SYSTEM.md` §4 correction (ready gate `FUN_80052338`). |
| MIS-11 | [ ] | Follow-up to S-4. If the byte is ever non-zero: `bp 0x8004F1A8` in that context and trace the caller chain back to the menu / sortie code (*EXE*). If never: `bp 0x8004F1A8` outside missions (assignment screen, arena, garage) and record which PA file each context opens. | The real selector for PA01–PA71. | `MISSION_STAGES.md` Part 1 "where the stage byte VALUE is set — OPEN"; closes the MISSION_STAGES/LEVEL_LOADING reconciliation. |
| MIS-12 | [ ] partial 2026-10-21 — objobj method table `+0x20..+0x2C` and resulting thinks: **M20** `0, 0x801C52C0, 0x801C52F0, 0x801C5604`; type 1 → think `0x801CA1A8`, type 3 → `0x801C53FC`. **M24** `0, 0x801C52C0, 0, 0`; type 1 → think `0x801C9648` (= matched twin of fdat000 `func_801C8CBC`, the enemy-AC think). **M28** `0, 0x801C64C0, 0x801C6508, 0`; thinks not read. The trap address `0x80078D50` is the `lw v0,0x20(v0)` that loads the method (`v0 = objobj + type*4`); the `jalr` follows. Other missions open | `bp 0x80078D50` (type dispatch) in every mission you visit; log `(mission, type=instance+0x0E, fn)`. | The concrete type→think map, to merge into `disc_map/type_catalog_long.csv`. | `ENTITY_TYPES.md` "Concrete numeric type→think for all 59 missions". |
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
| ENT-9 | [ ] partial 2026-10-21 — **destruction path CONFIRMED live** (mission 28, records 108 [HP 200] and 112 [HP 50]): stats `+0` → `0xFFFF`, bbox `+0x18..+0x24` → `0x7FFF`, handler `+0x2C` → 0, and (not in the doc) record byte `+0x06` `0x30` → `0xFF`. The per-hit decrement was not caught (the two watched props were not the ones destroyed). M28 has 77 structure records with stats: HP 200, 50 and one 300 | After S-5: `bp 0x801C58C4` (*objobj* on-hit) on a generator; read the hit record. | Which field is current HP for **world-structure** objects (`0x801D0B68` array) vs MTs. | `DESTRUCTIBLE_OBJECTS.md` discrepancy; HP of other generators; pulse-rifle damage value. |
| ENT-10 | [ ] | `M24-ctrl-01` (enemy AC slot 1 `0x801A2828`, think `0x801C9648` = twin of fdat000 `func_801C8CBC`): `ww 0x801A294C` (aligned word of node index `+0x126`) and `bp` its move handler (word at `+0x54`, `0x801A287C`) with `a1 == 9`; lead it toward a ledge. | The node sequence it walks; whether it ever leaves the `0x80198B08` boxes; what event 9 does (next-node pick?). | HYPOTHESIS from the matched think: enemy ACs follow authored node boxes and never sense ledges (why they don't fall); `AI_PATHS.md`; `0x80198B08` row stride (0x34 in the matched code vs 0x6c in `ENTITY_TYPES.md`). |
| ENT-11 | [ ] | `M28-ctrl-01`: `ww` a car's stats HP word (record 83: `0x801D9CDC`, HP 50, the 40×100 box ~900 units from the start) and walk onto the car without firing. | `pc`/`ra` of the HP write: the weapon route (`FUN_800835E8` world class) or a collision/contact path; damage per step. | Cars blow up when stepped on (seen live 2026-10-21); `COLLISION.md`, `DESTRUCTIBLE_OBJECTS.md` hit path. |

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
| LVL-6 | [ ] partial 2026-10-21 — missions **20, 24, 28 all read e200**, with the same pattern: (1,101)→`0x8004ADA0`, (2,202)→`0x8004ADA0`, (1,100)→`0x800B17C8`, **(1,N)**→`0x800B17C8` with N = the mission number (20/24/28), **(2,200)**→`0x800B17C8` (`ra 0x8004F5D4`), (2,2N)→`0x801C4B40`, (2,2N+1)→`0x800B74D8`, (0,1), then archive 0 entries 0, 2, 13, 49, 89, 60, 71, 108, 166 from `ra 0x8005365C`. Whether non-e200 missions skip the (2,200) read is the next check | Load e200-using missions: `bp FUN_8004F508` and log every `read_T_entry` (`bp 0x800165E4`). | How a mission selects a shared scene; e200's header. | `LEVEL_LOADING.md` §7 shared-scene mechanism + e200 header. |

---

## INT. Interactions / COM / save

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| INT-1 | [ ] | `ww 0x80150000` (first dialog-text word) and open a door / pick up an item. | `pc`/`ra` of the COM-dialog writer → the queue structure. | `INTERACTIONS.md` §3 "queue not yet RE'd". |
| INT-2 | [ ] | Dump the memory card before and after a mission in which a weapon is picked up (and once without). | Where the obtained part lands in the main `…A` save. | `INTERACTIONS.md` §2 storage HYPOTHESIS; starts the main-save format. |

---

## PER. Perception / stealth (`PERCEPTION_MODEL.md` §7 recipes)

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| PER-1 | [ ] | (P-2 remainder) Fill the gap `0x801CF340..0x801D0B3F` with a canary pattern in a mission with fewer units than mission 8, play through a cutscene/phase change, diff. | Every canary word intact ⇒ gap free in that mission too. | `PERCEPTION_MODEL.md` §4/§5 free-RAM claim beyond mission 8; §7 P-2. |
| PER-2 | [ ] | (P-3) Dump the active set `0x801A5DD8` (64 B) standing in room A, then in room B. | A∈set(B) ⇔ B∈set(A). | `PERCEPTION_MODEL.md` §2 (active set = coarse PVS, CONFIRMED static only); §7 P-3. |
| PER-3 | [ ] | (P-4) Mission 8 red-AC scene: write slot 2 `+0x48` (`0x801A29E0`) = 0 every frame for ~30 s. | No fire, no crash; the movement fallback. | `PERCEPTION_MODEL.md` §3 "Null target is a supported state" (CONFIRMED / HYPOTHESIS); §7 P-4. |
| PER-4 | [ ] | P-6 ghost build with the HP stores `0x800753C4/C8` **live**; blind the AIs and stand away from the ghost. | Rounds/missiles aimed at the ghost point (projectile spawns with the ghost as target). | `PERCEPTION_MODEL.md` §7 P-5/P-6 "fire at the ghost". |
| PER-5 | [ ] | Ghost build: get the player sighted, then move to a room whose section is not in the ghost's active set. | The aim cone rejects the ghost (section `+0x16`) ⇒ a room-change "they lose you" rule. | `PERCEPTION_MODEL.md` §7 P-5 "section `+0x16` / active-set interaction". |
| PER-6 | [ ] (candidate scene 2026-10-21: mission 20 slot 1 — type 1, think `0x801CA1A8`, HP 7900 — kept `+0x48` = player while the player was being shot at through a wall (seen on screen), stock omniscient acquisition as in §1a; `M20-ctrl-01` + walk forward ~80 s) | Ghost build in a mission with a ground MT/AC attacker: pin its ghost behind a wall. | Pathfinds around vs beelines into the wall. | `PERCEPTION_MODEL.md` §7 P-6 "non-flying AI class". |

---

## DMG. Mission damage events (`MISSION_DAMAGE_EVENTS.md` §6 recipes)

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| DMG-1 | [ ] | (D-1) Mission with a blade AC: `bp 0x8007A334` while it swings; read 28 B at `a2`. | The descriptor bytes; which `0x8008E530` entry they match. | `MISSION_DAMAGE_EVENTS.md` §2 "Minimum viable synthetic projectile" (HYPOTHESIS hooks). |
| DMG-2 | [ ] | (D-2) `bp` the struck NPC's `+0x5C` handler on a hit; single-step, log every `lbu/lhu/lw` off `a1`. | Max descriptor offset read. < 4 ⇒ the 4-byte descriptor is safe. | `MISSION_DAMAGE_EVENTS.md` §2 (HYPOTHESIS hooks). |
| DMG-3 | [ ] | (D-3) Freeze the scene; write desc `{10 00 C8 00}` to scratch RAM; point a frozen NPC's `+0x58` at a stub that calls `0x800835E8(0, slot2, desc, …)` once. | Slot 2 `+0x160` drops by exactly 200. | `MISSION_DAMAGE_EVENTS.md` §3 chosen mechanism. |
| DMG-4 | [ ] | Capstone-style disc: one authored spawn record with distinct `+0x12/+0x14/+0x16/+0x22..27` values, spawned on room entry. | What each field does. | `MISSION_DAMAGE_EVENTS.md` §6 spawn-record semantics. |

---

## CTL. Player control (`PLAYER_CONTROL.md` §5 recipes)

| ID | Status | Do | Look for | Settles |
|---|---|---|---|---|
| CTL-1 | [ ] | (C-1) Any mission: `bp 0x8004FA24` (player_init), then write `+0x28` (`0x801A26E0`) and `+0x2A` (`0x801A26E2`) to ¼; `ww 0x801A26E0`; play. | Passes under/through geometry it couldn't before? Anything restores the values? Do `+0x6A/+0x6C` (`0x801A2722/24`) follow? Camera clip? | `PLAYER_CONTROL.md` §4 collision-scale (OPEN). |
| CTL-2 | [ ] | (C-2) Mid-mission, write handler set B's pointers onto the set-A player. | Hot-swap works without re-init. | `PLAYER_CONTROL.md` §1/§3 "set H" mechanism. |
| CTL-3 | [ ] | (C-3) Scene load: `ww` slot 0 `+0x60` (`0x801A2718`). | `pc`/`ra` of the writer = how the garage AC model is assembled. | `PLAYER_CONTROL.md` §5 C-3 (Goal B prerequisite). |
| CTL-4 | [ ] | Toggle `0x8004121A` before `player_init` in a tank save and a biped save. | Whether it is the leg class. | `PLAYER_CONTROL.md` §5 last bullet. |

---

## Results log

Append one line per session: date · items closed · **state IDs archived** (from
`disc_map/states/STATES.tsv` via `tools/duckstation/states.py archive`; cite IDs such as
`M01-ctrl-01`, never slot numbers — slots are scratch) · docs updated.

- 2026-10-21 · closed **MIS-7, MIS-9**; partial **S-4** (arena left), **MIS-12, LVL-6, MIS-4** (refuted as a structure tally), **ENT-9** (destroy path) · states `M20-ctrl-01`, `M24-ctrl-01`, `M28-ctrl-01`, `M34-exit-01` · side data: after the player died in M28, `0x8019F524` = `0x600` · docs: MISSION_SCRIPT_VM, ENTITY_TYPES, MISSION_STATE_MACHINE, LEVEL_LOADING, MISSION_SYSTEM, DESTRUCTIBLE_OBJECTS · traps were armed only after `read_T_entry(2, 202)` returned (EXE `0x800165E4`), because the ovl202 addresses hold ovl201 code in the garage. Added tables PER/DMG/CTL for the stealth-doc recipes, ENT-10 (ledges), ENT-11 (stepped-on cars).
- 2026-10-18 · closed **S-1, S-3, S-5** (ENT-1 partial: MT position = s16 X/Y/Z at `+0x08/+0x0A/+0x0C`, written by `0x8004F024`; `0x8004121B` read 0 in mission 30 — S-4 side-data) · states `GAR-main-01`, `M30-ctrl-01`, `M30-mid-01` · docs: OBJECT_STATS, DESTRUCTIBLE_OBJECTS, COMBAT_PHYSICS, COLLISION, ENTITY_AI_FRAMEWORK, RUNTIME_RAM_MAP, AUTHORING, MISSION_SYSTEM · plan/notes: `disc_map/trace/LIVE_SESSION_1_PLAN.md`. Next quick win: shoot the hw8=3072 MT (slot 2, `0x801A2998`, HP@`0x801A2AF8`) to quantify hw8 as defense (MIS-1).
