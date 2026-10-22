# AC1 mission lifecycle — the state machine, consolidated

Target **SLUS-01323 (v1.1)**. This page pulls the mission runtime together as **one
state machine**: what loads, what ticks each frame, which variables carry the state,
who is allowed to write them, and how a mission ends. The per-topic evidence stays in
the source docs — this page cites them rather than repeating the disassembly:

- [MISSION_SYSTEM.md](MISSION_SYSTEM.md) — loader, spawn table, timer, result flags, primitives, live completion flow
- [MISSION_SCRIPT_VM.md](MISSION_SCRIPT_VM.md) — the secondary actor-thread VM (chunk 4)
- [ENTITY_TYPES.md](ENTITY_TYPES.md) — the objective object (FDAT `2N`), INIT/think binding, condition taxonomy
- [ENTITY_AI_FRAMEWORK.md](ENTITY_AI_FRAMEWORK.md) — the entity array and per-frame entity update
- [INTERACTIONS.md](INTERACTIONS.md) — doors, items, COM dialogs
- [DESTRUCTIBLE_OBJECTS.md](DESTRUCTIBLE_OBJECTS.md) — world-structure objects and the `0x400` event bit
- [REFERENCE.md](REFERENCE.md) §2 (game loop), §11 (PA loader), §12–13 (mission symbols)

Executable form: **`re/ac1_mission`** (`re/include/ac1_mission.h`, `re/src/ac1_mission.c`,
validator `re/examples/mission_sim.c`) — a host-side C model of §3–§6 below plus
byte-exact decoders for the chunk stream, spawn table and script blob.

Labels as elsewhere: **CONFIRMED** / **HYPOTHESIS**; open points cite
[LIVE_CHECKLIST.md](LIVE_CHECKLIST.md) IDs.

---

## 1. Where a mission sits in the game loop  CONFIRMED

```
FUN_800122A8 game loop (resident EXE)
  outer: wmemset(workspace); read_T_entry(FDAT, 201) ; FUN_8009B414()
  inner: switch DAT_80039C5E (game mode)          0 → FDAT 202 (mission overlay)
                                                  1 → FDAT 203 (link versus)
                                                  2 → FDAT 204 (local battle)
         (*DAT_8004ADA4)()                        ← current state handler (overlay entry)
         loop while result != 0
```

Mode **0 + overlay 202** is the mission runtime. Everything below runs inside that
overlay (base `0x8004ADA0`) except where marked *objobj* (the per-mission objective
object, FDAT `2N`, at `0x801C4B40`) or *EXE* (resident). Entering a mission from the
garage/SORTIE path and the post-mission debrief are *not* yet RE'd — the base EXE
consumes the result code `0x80048610` (§6) and that is where our trace stops
(open: who writes the mission number `0x800411F8` and the PA-pack byte `0x8004121B`
on the way in — LIVE_CHECKLIST **S-4 / MIS-11**).

---

## 2. LOADING — `FUN_8004F508`  CONFIRMED

Ordered steps (addresses in MISSION_SYSTEM.md §1, REFERENCE §11):

1. Init the **mission control block** `0x8019F51C..0x8019F532` (§3), store mission
   number `0x800411F8 → 0x8019F518`.
2. `read_T_entry(RTIM, 100)` and `read_T_entry(RTIM, N)` — common + per-mission TIM bank.
3. `read_T_entry(FDAT, 200)` — the shared stream.
4. `read_T_entry(FDAT, 2N) → 0x801C4B40` — the **objective object** (code + vtable).
5. `read_T_entry(FDAT, 2N+1) → *0x8019F520` — the **chunk stream**, then walk it:
   `[u32 len][payload]…`, fixed handler per chunk index (0–2 setup incl. local geometry,
   4 script blob, 7 section placement, 11 geometry records, **12 spawn table →
   `FUN_80073B74` → instance templates `0x8019FAB8` (256×44)**).
6. Call *objobj* **INIT** (`[*0x8019F51C+0x14]`).
7. Zero the displayed timer `0x8019F52C` (`0x8004F9FC`).
8. `FUN_8004F1A8` — load the PA pack selected by `0x8004121B` (observed **0 = PA00**, the
   common object/effect pack; the walkable level came in with chunk 0/7 in step 5 —
   see the reconciliation in [MISSION_STAGES.md](MISSION_STAGES.md)).
9. `FUN_80078B14` per-object setup — bind every live instance to its geometry block.

Transition **LOADING → RUNNING** is implicit: the state handler starts ticking the
mode-2 body.

---

## 3. The state — variables and their owners

| Variable | Addr | Written by | Read by | Status |
|---|---|---|---|---|
| objobj ptr | `0x8019F51C` | loader | driver, VMs, spawn dispatch | CONFIRMED |
| work/stream ptr | `0x8019F520` | loader | chunk handlers | CONFIRMED |
| **flags** | `0x8019F524` | `FUN_8004C318` (store), `FUN_8008A80C` (`|= / &= 0x2400`), `FUN_8008A8F8` (`&= ~0x1000` every frame), VM action bit / destructible action bit (`|= 0x400`) | exit (`0x8004C69C`), HUD | CONFIRMED (bits: `0x80` progress, `0x100` success, `0x200` fail, `0x400` event, `0x1000` timer-warn, `0x2400` objective-pending) |
| **end counter** | `0x8019F528` | `FUN_8004C318` arms to 100 if 0 | driver → `FUN_8008AA90` counts to 0 → exit | CONFIRMED |
| **end-anim / timer-mode** | `0x8019F52A` | `FUN_8008A80C` (0xFFFF / 0), `FUN_8008A8F8` (decrement; 60 = special) | driver (`< 61` runs the state machine), HUD (`< 16` shows timer) | CONFIRMED |
| **displayed timer** | `0x8019F52C` | cmd 4 (`22 × param`), loader (0), map change (0) | HUD MM:SS; timer-tick → FAIL on expiry | CONFIRMED vars / HYPOTHESIS units (**MIS-2**) |
| `+0x12/+0x14/+0x16` | `0x8019F52E..32` | loader (128 / 0 / 64) | ? | partial |
| **progress counter** | `0x8009079C` | `FUN_8008A80C` (`++`, terminal at 36) | objobj ticks (M0/M3 intro sequencer) | CONFIRMED |
| terminal flag | `0x801CC80C` | `FUN_8008A80C` at 36 | ? | CONFIRMED (setter) |
| **objectives-complete** | `0x801D0B50` (= timer struct `0x801D0B40`+0x10) | destructible / objective logic (**3** = all objectives done) | border-exit gate in the objobj tick | CONFIRMED live; relation to `0x8009079C` open (**MIS-4**) |
| timer struct | `0x801D0B40` | cmd 4 helper (`+0x44/+0x46`), per-frame countdown | alarm beeps `FUN_800529F0`, FAIL on expiry | CONFIRMED |
| **result code** | `0x80048610` | exit (`0x8004C69C`) | base EXE | CONFIRMED |
| VM blob ptr / thread table | `0x801D0B64` / `0x801D0B60` (+`B50` set idx, `B51` count) | chunk-4 stager (**MIS-7**), `vm_load_threads` | `vm_tick` | CONFIRMED |
| instance templates | `0x8019FAB8` (256×44) | `FUN_80073B74` at load; binder marks consumed | spawn, binder | CONFIRMED |
| entity array | `0x801A26B8` (stride `0x170`, slot 0 = player) | allocator `0x80078A2C`, INIT ctor, thinks | everything | CONFIRMED (slot count open, **ENT-3**) |
| world-structure array | `0x801D0B68` (stride `0x40`) | chunk handlers, on-hit handlers (retire with `0x7FFF`) | collision, render, interact | CONFIRMED |

**Who may commit a result:** only `FUN_8004C318`. Its three callers are the timer-tick
(`0x200`), primary-VM cmd 5 (`0x80`) / cmd 8 (`0x100`, prompt-gated), and the
secondary-VM `set_result` opcode (`0x100A`, data-driven `0x80/0x100/0x200`). No M0–M3
objective tick commits `0x100` itself (ENTITY_TYPES.md).

---

## 4. RUNNING — one frame  CONFIRMED order

```
mode-2 handler body (~0x8004C3xx), every frame
 ├─ 0x8004C480  FUN_8008AB68  mission driver
 │    1. [*0x8019F51C+0x00](objobj)     objective TICK (per-mission code)
 │    2. if 0x8019F52A < 61 → FUN_8008A8F8   end-anim state machine + 0x1000 masker
 │    3. if 0x8019F528 != 0 → FUN_8008AA90   end-sequence step (fade / flash, → exit at 0)
 ├─ ~0x8004C4F0  timer tick: countdown struct; thresholds → beeps; expiry → FUN_8004C318(0x200)
 ├─ 0x8008B380  vm_tick: secondary VM — per running thread, dispatch loop 0x8008B42C until
 │              it blocks (set_loop / mark_lerp) or END; blocked threads run their lerp tail
 ├─ entity update passes (ENTITY_AI_FRAMEWORK.md §1, COMBAT_PHYSICS.md §6):
 │    per entity: +0x4C wrapper → +0x50/+0x54/+0x58/+0x5C methods (null-checked),
 │    generic updater ~0x800789D0 (think +0x58, then store pos → +0x20/+0x24)
 ├─ projectiles (round think FUN_80083A10 → collision → FUN_800835E8 damage dispatch)
 └─ HUD (timer MM:SS when 0x8019F52A < 16 && timer != 0; "#Location" blip)
```

The primary command VM `FUN_8008A0B0` is **not** a per-frame interpreter: it is a
10-entry dispatcher the objective tick calls with `cmd` (1..10) when its own condition
logic decides to (set timer, load a thread set, commit progress, …).

---

## 5. The state machine

```
            FUN_8004F508
  ┌──────┐  load+walk+INIT  ┌─────────┐
  │ LOAD ├─────────────────►│ RUNNING │◄──────────────────────────────┐
  └──────┘                  └────┬────┘                               │
                                 │ objobj / destructible logic         │ (no commit yet:
                                 │ sets 0x801D0B50 = 3                 │  COM "last objective",
                                 ▼                                     │  flags untouched)
                       ┌─────────────────────┐                         │
                       │ OBJECTIVES_COMPLETE ├─────────────────────────┘
                       └──────────┬──────────┘
                                  │ player crosses the exit border
                                  │ → objobj tick → cmd 5 → FUN_8004C318(0x80)
                                  ▼
      timer expiry ───────► ┌──────────┐ ◄─── secondary-VM set_result(0x100/0x200)
      FUN_8004C318(0x200)   │  ENDING  │ ◄─── cmd 8 prompt-gated FUN_8004C318(0x100)
                            │ (fade,   │
                            │ 0x8019F528│
                            │ 100 → 0) │
                            └────┬─────┘
                                 │ counter hits 0 → 0x8004C69C → result code 0x80048610
                                 ▼
                            ┌──────────┐   base EXE: debrief ("Income and expense report")
                            │   EXIT   ├─► or retry/fail flow. Not RE'd past here.
                            └──────────┘
```

Transitions and their evidence:

| From → To | Trigger | Writer | Evidence |
|---|---|---|---|
| LOAD → RUNNING | loader returns | — | MISSION_SYSTEM §1 |
| RUNNING → OBJECTIVES_COMPLETE | last objective destroyed / condition met | `0x801D0B50 = 3` (objobj / destructible code) | **live** (MISSION_SYSTEM §6): flags, result code and objobj all stay 0 at this point |
| OBJECTIVES_COMPLETE → ENDING | exit border crossed | objobj tick → cmd 5 → `FUN_8004C318(0x80)`; live read `0x480` | **live** (§6); `0x400` setter open (**MIS-5**) |
| RUNNING → ENDING (fail) | timer reaches 0 | timer-tick → `FUN_8004C318(0x200)` | disasm (§3/§4) |
| RUNNING → ENDING (scripted) | script thread executes `set_result` | secondary VM `0x100A` → `FUN_8004C318(flags)` | disasm (MISSION_SCRIPT_VM) |
| RUNNING → ENDING (prompt) | cmd 8 and `FUN_80052A2C(99) == 1` | `FUN_8004C318(0x100)` | disasm; trigger open (**MIS-3**) |
| ENDING → EXIT | `0x8019F528` counts 100 → 0 | `FUN_8008AA90` | **live** (~100-frame fade) |
| EXIT → debrief | base EXE reads `0x80048610` | `0x8004C69C` | disasm |

Sub-machines that run *inside* RUNNING:

- **Objective progress** (`FUN_8008A80C`): `0x8009079C` steps 0→36; each step sets
  `end_anim = 0xFFFF`, `flags |= 0x2400`, plays jingle 101 (not on the first step);
  step 36 is terminal (`end_anim = 0`, `0x801CC80C = 1`, `flags &= ~0x2400`). M0/M3
  use it as an intro sequencer; it never commits a result.
- **End-anim state machine** (`FUN_8008A8F8`): active while `0x8019F52A < 61`;
  state 60 fires the special end action (`DAT_80039C5D` callback) and clears `0x1000`;
  otherwise decrements toward 0. Every frame it also clears the transient `0x1000`.
- **Secondary VM threads** (MISSION_SCRIPT_VM.md): per thread
  `IDLE → RUNNING (dispatch) → BLOCKED (set_loop n frames / mark_lerp) → RUNNING … → END`.
  `spawn_group N` is what makes enemies "appear out of nowhere" mid-mission: template N
  → `0x80078CFC` → allocator → resource load → binder → **INIT[type]** installs the
  entity vtable incl. `+0x58`.
- **Entity AI** (per class, *objobj* thinks): state byte `+0x40`/`+0x41`
  (COMBAT_PHYSICS §6 state machine `0x801C75BC` for ACs; `0xFF` = dead).

---

## 6. EXIT — the result code  CONFIRMED (table) / open (meaning of `0x80`)

`0x8004C69C` maps `0x8019F524 → 0x80048610`: bit `0x100` → **1** (success), bit
`0x200` → **2** (fail), neither → **0**. The live border exit committed `0x80`
(+ `0x400`), which this table maps to **0** — yet the game proceeded to the income
report as a success. Either the debrief treats 0 as "completed, score by flags" or
there is a second success path we have not seen. **MIS-16** reads `0x80048610` and the
base-EXE consumer right after a border exit. Related: `FUN_8004C318` is a *store*
per the disasm, which would clobber `0x400` on a later `0x100` — but `0x480` was read
at exit; **MIS-15** decides store vs OR (it changes authoring semantics for
`set_result`).

---

## 7. Spawn lifecycle (data → entity)  CONFIRMED

```
chunk 12 record (40 B, 20×int16)          ── FUN_80073B74 ──►  template 0x8019FAB8+i*44
  hw0..2 pos, hw3 block/type, hw5 rot,         +0x00 = 0, +0x04+2K = hwK, +0x02 = hw11
  hw7 resource id, hw11 HP
                                            ── spawn_group N / objobj EVENT ──► FUN_80078CFC
                                            ── or FUN_80078F00 streaming ──► FUN_80078CFC
                                               (template section +0x12 enters the visible-
                                                section mask; flags +0x14 bit 0 = zone-box
                                                test instead, 0x40 = never auto-spawn)
                                            ── 0x80078A2C allocator ──► entity slot (stride 0x170)
                                            ── FUN_800788C8 resources ─► geometry/collision
                                            ── FUN_80078B14 binder ────► copy template, geom record
                                                                         (+0x0A → 0x8019F538+idx*44),
                                                                         zero +0x4C..+0x5C, mark consumed
                                            ── 0x80078D50 dispatch ────► INIT[type = +0x0E](entity)
                                                                         installs +0x50..+0x5C (think)
```

The streaming path is CONFIRMED live in mission 20 (`LIVE_CHECKLIST.md` MIS-9: templates 5 and 14 spawned ~80 s after load from `ra 0x8007908C`). The flag-bit meanings are read from its byte-matched C Live 2026-10-21: every `0x40` template seen (mission 28: records 6/7/8/12) was spawned only by objobj code, never by the streamer; zone mode (bit 0) not yet seen live. Mission 28's waves respawn the same templates after kills.

Despawn: `despawn_group N` → `FUN_80078D78`; death: on-hit/death method `+0x5C`;
*objobj* **FREE** hook (`+0x10`) adjusts kill tallies. World-structure objects
(doors, generators) live in the separate `0x801D0B68` array and retire by nulling
their `+0x2C` handler and writing `0x7FFF` sentinels (DESTRUCTIBLE_OBJECTS.md).

---

## 8. What is still open (and where it is tracked)

| Open point | Checklist |
|---|---|
| Who sets the mission number / PA-pack byte on the way in (garage → SORTIE) | S-4, MIS-11 |
| Timer unit and the `×22` scale; countdown-struct ↔ displayed-timer relation | MIS-2 |
| cmd 8 prompt trigger | MIS-3 |
| `0x801D0B50` vs `0x8009079C` (two counters or one?) | MIS-4 |
| Setter of `0x400` | MIS-5 |
| `FUN_8004C318` store vs OR; meaning of `0x80` at exit / result code 0 | MIS-15, MIS-16 |
| Secondary-VM thread order, `init_unit` / `mark_lerp` / `op_f` fields, set-header flags | MIS-6 |
| Chunk-4 stager (writer of `0x801D0B64`) | MIS-7 |
| Missions 20/24/28 without chunk 12 | MIS-9 |
| READY-GATE owner `0x801FD844` | MIS-10 |
| Type→think map for all 59 missions; AUX_TBL bits | MIS-12, MIS-13 |
| Primary-VM cases 00/01/02/09 | MIS-14 |
| Entity slot count; `+0x58` role per class; integrator | ENT-1..3, S-3 |
| COM message queue; item persistence; debrief / save path | INT-1, INT-2 |

Static work that does **not** need the emulator and would extend this page:
decompile the objective objects of more missions (ENTITY_TYPES.md has M0–M3) to
grow the condition taxonomy and the type→think table; trace the base-EXE
garage→SORTIE path for the `0x800411F8` / `0x8004121B` writers; read the chunk-4
stager out of the lower overlay block.
