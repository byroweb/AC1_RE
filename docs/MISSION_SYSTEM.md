# AC1 Mission Runtime — descriptor, MT spawn, timer, objectives

Target **SLUS-01323 (v1.1)**. Code-side companion to [REFERENCE.md](REFERENCE.md) §11/§12,
[PA_HEADER.md](PA_HEADER.md) (slot binding) and [PA_FORMAT.md](PA_FORMAT.md). All addresses are in
the **mission overlay = FDAT.T entry 202 (0xCA)**, loaded to base `0x8004ADA0`
(extract with `tools/extract/extract_t.py`; not in the entry-201 Ghidra DB). Offsets
verified on the **pristine** bin. Tool: `tools/mission/mission_parse.py`.

Notation: `CONFIRMED` = byte/disasm-verified here. `HYPOTHESIS` = inferred, not yet
ground-truthed in DuckStation.

---

## TL;DR architecture (CONFIRMED)

Each mission **N** is described by a PAIR of FDAT.T entries (file id 2):

| FDAT entry | Role | Loaded to |
| --- | --- | --- |
| **`2*N`**   | **objective object**: relocatable MIPS **code+data**, starts with a method **vtable** | `0x801c4b40` (`= *0x8019F51C`) |
| **`2*N+1`** | **chunk stream**: `[u32 len][payload]…` consumed by the scene loader `FUN_8004F508` | work RAM `0x800B17C8` / `*0x8019F520` |

Confirmed by entry-size parity (even entries small + start with a `0x801c….`
vtable; odd entries large geometry/instance streams) for entries 0–21.

The **objective LOGIC is per-mission MIPS code** (the `2*N` object's vtable
methods), NOT a single global objective-type enum/switch. Those methods call back
into shared overlay **primitives** (set timer, advance objective, set
success/fail flag). The shared runtime owns the timer, the HUD, and the
complete/failed flag word; each mission decides *when* to call the primitives.

---

## 1. Mission scene loader — `FUN_8004F508` (`0x8004F508`)  CONFIRMED

Entered on mission start. Prologue inits the **mission control block** at
`0x8019F51C` (see §5), stores the mission number byte `DAT_800411F8` →
`0x8019F518`, then:

```
read_T_entry(1, 100,            dest=0x800B17C8)   ; RTIM.T common
read_T_entry(1, missionNum,     dest=0x800B17C8)   ; RTIM.T per-mission
read_T_entry(2, 200,            dest=*0x8019F520)  ; FDAT entry 200 (shared stream)
read_T_entry(2, missionNum<<1,      dest=0x801C4B40)  ; FDAT 2N  -> objective object
read_T_entry(2, (missionNum<<1)|1,  dest=*0x8019F520) ; FDAT 2N+1 -> chunk stream
```

It then **walks the chunk stream** (`s1`), each step `len = u32@s1; handler(s1+4);
s1 += len+4`, dispatching chunks to fixed handlers (CONFIRMED order):

| chunk | handler | meaning |
| --- | --- | --- |
| 0–2 | `FUN_80053848` | (3 setup chunks) |
| 3 | `FUN_80057994` | |
| 5 | `FUN_8004C854` | -> `*0x8019F520` |
| 6 | `FUN_8006D9DC` | |
| 7 | `FUN_80071E14` | |
| 8 | `FUN_8007334C` | |
| 9 | `FUN_800731E8` | |
| 10 | `FUN_80056AD0` | |
| 11 | `FUN_800739AC` | |
| **12** | **`FUN_80073B74`** | **OBJECT-INSTANCE / MT-SPAWN table** |

After the walk it calls `[*0x8019F51C + 0x14]` (the objective object's **init**
method), zeroes the displayed timer `0x8019F52C`, calls `FUN_8004F1A8` (the PA
stage loader — see REFERENCE §11), then `FUN_80078B14` (per-object setup, binds
each instance to PA geometry).

---

## 2. MT / object spawn table  CONFIRMED

Chunk 12 of the `2*N+1` stream = **256 records × 40 bytes** fed to
`FUN_80073B74` (`0x80073B74`), which fills the **instance template table
`0x8019FAB8`** (256 × 44). Per source record (20 int16 halfwords) it writes:
`rec[+0]=0`, copies the 20 halfwords to `rec[+0x04..+0x2B]`, then `rec[+0]=rec[+0x18]`.

Source 40-byte record (`hwK` = int16 at byte 2K):

| field | meaning | status |
| --- | --- | --- |
| `hw0,hw1,hw2` | X, Y, Z position | CONFIRMED |
| `hw3` | **geometry block index** (`-> instance +0x0A`; `-1` = none) — binds to PA block via `0x8019F538` (PA_HEADER §"per-object setup") | CONFIRMED |
| `hw5` | rotation (PSX angle units; `2048` = 1/16 turn observed) | HYPOTHESIS |
| `hw7` | **object / MT TYPE id** | CONFIRMED (varies per mission: 5,7,8,25,33,34,62,190,196…) |
| `hw8..hw19` | per-type params (HP / flags / link / area ids) | HYPOTHESIS |

Empty slots carry `hw3 == -1` and zero position. Mission 0 has 6 non-empty
records; mission 2 has different types/positions. Binding to geometry & display
slot is `FUN_80078B14` (`+0x0A` = block index → 44-byte record table
`0x8019F538`; `÷44` magic = display-slot 1..16). Dump:
`tools/mission/mission_parse.py --stream <2N+1>.bin --spawns`.

> Note: this is the **placement/spawn list**. Whether AI/HP live here (hw8..hw19)
> or are looked up by `hw7` (type id) in a separate stat table is not yet
> confirmed — best next ground-truth target.

---

## 3. Mission timer system  CONFIRMED (vars + driver) / HYPOTHESIS (units)

### Displayed timer `0x8019F52C` (`-2772` off `0x801a0000`)
- HUD draw `0x8005075C`/`0x80050794`: shown as **MM:SS** via the classic
  ÷3600 / ÷60 frame-magic (`0x2E8BA2E9`). Drawn only when the timer-mode flag
  `0x8019F52A < 16` and `0x8019F52C != 0`. The "MISSION TIMER" label string is at
  `0x8004ADB0`. CONFIRMED.
- Set by mission-script **command 4** (`0x8008A178`) and helper
  `FUN_8008A778` (`0x8008A778`): `timer = 22 * param` (HYPOTHESIS on the ×22 /
  unit); helper also writes `0x801D0B46`/`0x801D0B44` (a timer-display struct).
- Zeroed at mission start (`0x8004F9FC`) and on map change (`0x800788AC`).

### Timer struct `0x801D0B40` + countdown handler (`~0x8004C4F0`)  CONFIRMED
The per-frame in-mission update (`~0x8004C3xx`, mode-2 handler body) ticks a timer
struct at `0x801D0B40` (`+0x00` u16 countdown, `+0x02`=1200, `+0x04` byte type,
`+0x06` u16 extra, `+0x10`=0xFF). Alarm beeps fire via `FUN_800529F0` when the
struct hits thresholds. **On expiry it sets the FAILURE flag** — see §4.

### `0x8019F52A` (`-2774`) = mission-END sequence counter  CONFIRMED
Separate from the displayed timer. The timer **state machine `FUN_8008A8F8`**
(`0x8008A8F8`, run each frame by the driver when `0x8019F52A < 61`):
- state `== 60` (`0x3C`): special end action (`DAT_80039C5D` callback, clears bit
  `0x1000` in `0x8019F524`);
- else: decrements `0x8019F52A` toward 0 (the mission-end animation frame count).

### Per-frame driver `FUN_8008AB68` (`0x8008AB68`)  CONFIRMED
Called once per frame from the mission update (`jal 0x8008AB68` @ `0x8004C480`):
1. calls `[*0x8019F51C + 0]` — the objective object's per-frame method;
2. if `0x8019F52A < 61` → `FUN_8008A8F8` (timer/end state machine);
3. if `0x8019F528 != 0` → `FUN_8008AA90` (end-sequence step + screen flash).

---

## 4. Objective / success–failure state  CONFIRMED

### The result flags word `0x8019F524` (`-2780`)  CONFIRMED
A 32-bit flag word in the mission control block. Mission exit reads it
(`0x8004C69C`) and produces the **result code** at `0x80048610`:

| `0x8019F524` bit | result code `0x80048610` | meaning |
| --- | --- | --- |
| `0x100` | `1` | **MISSION SUCCESS / COMPLETE** |
| `0x200` | `2` | **MISSION FAILED** |
| (neither) | `0` | (continue / debug "phase" bump `'0'`→`'1'` on `DAT_800411F8`) |

`0x80048610` is the value the **base EXE** reads to pick post-mission flow (it is
not written elsewhere in the overlay). CONFIRMED.

### Mission-end primitive `FUN_8004C318(a0)` (`0x8004C318`)  CONFIRMED
Writes `a0` directly into `0x8019F524`, and if the end counter `0x8019F528 == 0`
sets `0x8019F528 = 100` (the ~100-frame end fade). Callers / values:

| caller | `a0` | meaning |
| --- | --- | --- |
| `0x8004C5F0` (timer-tick handler) | `0x200` | **FAIL on timeout** |
| cmd 5 `0x8008A20C` | `0x80`  | generic end/objective flag |
| cmd 8 `0x8008A274` (after `FUN_80052A2C(99)==1`) | `0x100` | **SUCCESS when condition 99 holds** |
| `0x8008BAF8` (secondary script VM) | `hw` from script stream | **data-driven** flag set (can set 0x100/0x200) |

So **success = `0x8019F524 |= 0x100`, failure = `|= 0x200`**, both routed through
`FUN_8004C318`; the timer-tick path forces `0x200` on expiry.

### Objective-step primitive `FUN_8008A80C` (`0x8008A80C`)  CONFIRMED
Drives the **objective progress counter `DAT_8009079C`** (range-checked `< 37`):
- `progress == 36` (`0x24`): terminal — `0x8019F52A = 0`, sets `0x801CC80C = 1`,
  clears bit `0x2400` in `0x8019F524`;
- `progress < 36`: starts end sequence (`0x8019F52A = 0xFFFF`), sets bit `0x2400`,
  plays a jingle `FUN_800529F0(0,0,101)` when `progress != 0`, then
  **increments** `DAT_8009079C`.

Each call advances the mission one objective step; reaching 36 finalizes it. The
per-mission objective object calls this (e.g. mission-0 object method `0x801C5594`
is a thin wrapper around `FUN_8008A80C` callee `FUN_8008A80C`).

### Mission script command interpreter `FUN_8008A0B0` (`0x8008A0B0`)  CONFIRMED
Opcode in `a0` (1..10) indexes a **10-entry jump table at `0x8004C164`**:

| cmd | handler | action (CONFIRMED unless noted) |
| --- | --- | --- |
| 1 | `0x8008A10C` | clear a method-table flag on a range of objects |
| 2 | `0x8008A158` | call `FUN_8008C2FC` |
| 3 | `0x8008A168` | call `FUN_8008A048` (toggle group enable) |
| 4 | `0x8008A178` | **SET MISSION TIMER** (`0x8019F52C = 22*param`, type 128/256/512) |
| 5 | `0x8008A1E8` | end + `FUN_8004C318(0x80)` |
| 6 | `0x8008A21C` | enable group + counter (`FUN_8008A080`) |
| 7 | `0x8008A248` | group + counter |
| 8 | `0x8008A260` | if `FUN_80052A2C(99)==1` → `FUN_8004C318(0x100)` (**SUCCESS**) |
| 9 | `0x8008A284` | indirect call `[*0x8019F51C+8](idx)` (objective-object method) |
| 10 | `0x8008A2A8` | broadcast a `0x8200`+sub message to a range of objects |

`FUN_8008A080` increments a per-object kill/visit counter at `0x80031A94 + id`
(HYPOTHESIS: objective progress per object).

### Distance / "reach-location" HUD (radar)  CONFIRMED (display) / HYPOTHESIS (logic)
In the HUD (`0x80050960`–`0x80050AAC`): when `DAT_800411FB & 3 == 1`, it computes
`|player − target|` between player position `0x801A26C0/26C4` and target coords
`DAT_800411D6/11DA/11DE/11E2`, capping at 3 — a **location/objective blip**. The
"#Location Now" string at `0x8004C1AC` is rendered by a related path
(`0x8008D5A0`). The actual reach-area *success* still flows through
`FUN_8004C318(0x100)` / the objective object, not this HUD code.

---

## 5. Mission control block `0x8019F51C`  CONFIRMED layout

| addr | off | field | status |
| --- | --- | --- | --- |
| `0x8019F51C` | -2788 | ptr to active **objective object** (`= 0x801C4B40`); `+0/+4/+8/+0x14` = vtable methods called by the runtime | CONFIRMED |
| `0x8019F520` | -2784 | chunk-stream / work-buffer dest ptr (reused) | CONFIRMED |
| `0x8019F524` | -2780 | **result/objective FLAGS** (0x100=success,0x200=fail,0x400,0x1000=timer-warn,0x2400=objective-pending) | CONFIRMED |
| `0x8019F528` | -2776 | **mission-END sequence frame counter** (set to 100 by `FUN_8004C318`; counts to 0 → exit) | CONFIRMED |
| `0x8019F52A` | -2774 | timer-mode / end-anim counter (state 60 special; `<16` shows timer) | CONFIRMED |
| `0x8019F52C` | -2772 | **displayed MISSION TIMER** (frames; MM:SS on HUD) | CONFIRMED |
| `0x8019F52E` | -2770 | init 128 | partial |
| `0x8019F530` | -2768 | init 0 | partial |
| `0x8019F532` | -2766 | init 64 | partial |

Other shared symbols: `DAT_8009079C` = objective step counter; `0x80048610` =
mission result code; `DAT_800411F8` = mission number / phase byte;
`0x801D0B40` = timer-display struct; `0x801CC80C` = objective-terminal flag.

---

## 6. Live ground-truth — completion flow & spawn chain (DuckStation, 2026-06-14)

Everything in §4 was static (disasm) RE; this section **live-confirms** it on
hardware-accurate emulation and adds the observed end-to-end flow. Captured with
save states + watchpoints + idle-baseline diffs (see the `mem-diff-baseline`
skill). Ghidra names below are in the **isolated `Mission202` project** — see
`overlays/OVERLAY_MAP.md`.

### Level completion is **two halves** — the kill is not enough  CONFIRMED
1. **Destroy the last objective** → the field at **`0x801D0B50`** (= the mission
   timer/state struct `0x801D0B40` **+0x10**, §5) becomes **`3`** =
   *objectives-complete*, and a non-pausing **`COM : …` "last objective"** dialog
   fires. This does **NOT** set success or end the mission — verified:
   `0x8019F524`, `0x80048610` and the objective object all stayed `0` across the
   kill. (Open: reconcile `0x801D0B50` with the objective step counter
   `0x8009079C` — likely distinct fields.)
2. **Cross the level-exit border** (gated on the objectives-complete state) → the
   objective object's per-frame update runs script **cmd 5** (`0x8008A1E8`) →
   **`FUN_8004C318(0x80)`** writes `0x8019F524` and arms `0x8019F528 = 100`, the
   ~100-frame countdown that transitions to the **debrief / "Income and expense
   report"** screen. Live-observed `0x8019F524 = 0x480` at the exit = `0x80`
   (cmd 5) + the `0x400` bit already noted in §5 (its setter not yet pinned).

### Per-frame masker defeats a naïve watchpoint  CONFIRMED
`FUN_8008A8F8` (the state machine, §3) re-writes `0x8019F524` **every frame**
(`lw; and ~0x1000; sw` at **`0x8008A938`**) to clear the transient timer-warn bit
`0x1000`. A plain write-watchpoint on `0x8019F524` therefore traps every frame and
is useless. To catch the real setter (`FUN_8004C318`), **NOP `0x8008A938`** so the
word is no longer written each frame, trap the clean setter, then restore the
bytes (`24 f5 22 ac`). DuckStation patches the overlay with no I-cache problem.

### Dynamic mid-mission spawn — activation chain  CONFIRMED
The MT-spawn **table** (§2) is just placement data; enemies appear "out of
nowhere" when a **script opcode activates** a spawn record. Live chain:
spawn-group opcode (secondary VM dispatch `0x8008B42C` / handler `0x8008B830`,
distinct from the `0x8004C164` VM) → **`0x80078CFC`** spawn routine →
**`0x80078A2C`** slot allocator (AC array `0x801A26B8`, stride `0x170`) →
**`FUN_800788C8`** resource loader (geometry/collision via `FUN_80050FC4`) →
**`~0x80078C00`** record initializer → **`0x80078B14`** binder (marks the spawn
record consumed). The trigger is a script/location condition, **not** a collidable
trigger volume.

### Ghidra (Mission202 project) — names applied this pass
`mission_exit_commit` `0x8008A0B0` (the §4 script VM dispatcher; case bodies
`0x8008A10C–0x8008A2A8`), `mission_vm_jumptable` `0x8004C164` (10 entries, defined
as a JumpTable so the switch decompiles), `mission_set_result` `0x8004C318`,
`mission_eventflags_tick` `0x8008A8F8`, `mission_phase_dispatch` `0x8008AB68`.
`0x8004C318` and the table sit in the imported **`ovl202_lower`** block
(`0x8004ADA0–0x8004FFFF`). Player-interaction handlers (door/item) are in the
objective overlay — see **`docs/INTERACTIONS.md`**.

---

## CONFIRMED vs HYPOTHESIS summary

**CONFIRMED**
- Mission N = FDAT entry pair (`2N` objective-object code+vtable @0x801C4B40,
  `2N+1` chunk stream); scene loader `FUN_8004F508`; chunk 12 = 256×40 MT-spawn
  table → `FUN_80073B74` → `0x8019FAB8`.
- Spawn record: pos `hw0-2`, geometry block `hw3` (→ +0x0A binding), type `hw7`.
- Timer vars `0x8019F52C` (display) / `0x8019F52A` (end-anim) / struct `0x801D0B40`;
  driver `FUN_8008AB68`; state machine `FUN_8008A8F8`; HUD MM:SS path.
- Result flags `0x8019F524` (0x100=success / 0x200=fail) → result code `0x80048610`;
  end primitive `FUN_8004C318`; objective-step primitive `FUN_8008A80C`
  (counter `0x8009079C`, terminal at 36); script VM `FUN_8008A0B0` + jump table
  `0x8004C164` (cmd4=set-timer, cmd8=success-if-99, cmd5=0x80, timer-tick=0x200).

**HYPOTHESIS / open**
- Spawn `hw8..hw19` semantics (HP / AI / link / area ids) and whether stats are
  keyed by `hw7` type id in a separate table.
- The cmd4 `×22` timer scale / exact frame unit.
- The objective-TYPE distinction (survive / reach / defend / destroy) is encoded
  in the per-mission objective object's CODE and/or the secondary script VM
  (`FUN_8008B…`, opcode at `0x8008BAF8` sets flags from a data stream) — the
  enumerated condition checks (target-dead / player-in-area / timer / escort-alive)
  live there; not yet enumerated per opcode.
