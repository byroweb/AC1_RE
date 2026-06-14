# AC1 overlay map: who occupies `0x80050000–0x800DFFFF` and how to tell them apart

**Game:** Armored Core 1, North American `SLUS-01323` v1.1.
**Author:** AC1mod RE. **Last updated:** 2026-06-14.

## TL;DR / warning
The PS1 RAM range **`0x80050000–0x800DFFFF`** is a **swappable code-overlay region**.
Multiple unrelated FDAT.T overlays load to the **same base `0x80050000`** at different
times. **An address like `0x8008B830` does not identify code by itself** — you must
also know *which overlay was resident*. Annotating these addresses in the wrong
program produces nonsense.

- The **always-resident main exe** `0x80011000–0x80039FFF` is NOT swapped and matches
  Ghidra everywhere; resident-exe addresses are unambiguous.
- Everything **`>= 0x80050000`** is overlay-relative and ambiguous without context.

Convention going forward: **prefix every overlay-region symbol/comment with the
overlay tag** (`MISSION-202:`, `UI-201:`, `LINK-203:`, `ARENA-204:`). The
mission import already does this.

## The overlay set (FDAT.T entries, all load to base `0x80050000` = `0x8004ADA0`+hdr)
Per memory `project_ac1_mxt_loader`. The on-disc entry payload is the runtime image
(no relocation; only a trailing checksum word is stripped).

| FDAT entry | hex  | role                         | resident when                 |
|-----------:|------|------------------------------|-------------------------------|
| 201        | 0xC9 | front-end / garage / UI      | menus, garage, hub            |
| **202**    | 0xCA | **in-mission runtime**       | **a mission is playing**      |
| 203        | 0xCB | link-cable versus            | 2-console link battle         |
| 204        | 0xCC | local 2-AC arena / vs-CPU    | local arena                   |

Only ONE is resident at a time. **The existing Ghidra project
(`AC_1_USA_2048_RE`) contains the entry-201 (UI/garage) build** in this region — so
its `0x80050000+` functions are UI code, NOT mission code.

## Byte-mismatch evidence (proves 201 != 202 in this region)
Address **`0x80083894`**, same RAM address, two different overlays:

| source                              | bytes        | instruction          |
|-------------------------------------|--------------|----------------------|
| existing Ghidra program (overlay 201)| `27 bd ff e0`| `addiu sp,sp,-0x20`  |
| live mission RAM / this 202 dump    | `af b7 00 14`| `sw s7,0x14(sp)`     |

Verified directly in the 202 dump at file offset `0x33894` (= `0x80083894 − 0x80050000`):
`afb70014`. This is a different function prologue → different code → different overlay.

A second independent confirmation: the 202 dump's spawn-group opcode at `0x8008B830`
disassembles exactly to the documented `s1 = 0x8019FAB8 + N*44; jal 0x80078CFC`
handler (see below); the UI overlay has unrelated code there.

## How to tell which overlay is loaded (quick checks)
1. **Byte probe `0x80083894`:** `af b7 00 14` = **mission 202**; `27 bd ff e0` = **UI 201**.
2. **Overlay ABI** (`project_ac1_mxt_loader`): word at `0x8004ADA4` is the per-frame
   main-entry fn ptr. mission = `0x8004C340`, 203 = `0x8004BF94`, 204 = `0x8004BF3C`.
3. In a live DuckStation session, only overlay 202 has the MT-spawn table populated at
   `0x8019FAB8` and the timer at `0x8019F52C` actively ticking.

---

## What was created for the MISSION-202 overlay (this task)

### New, isolated Ghidra program
- **Project:** `/home/byron/Desktop/AC_1_USA_RE/ghidra_mission202/Mission202.gpr`
  (a brand-new, standalone Ghidra project — NOT the live `AC_1_USA_2048_RE`/`FARSI`
  projects, which were left untouched).
- **Program inside it:** `mission_overlay_FDAT202_80050000.bin`
- **Source bytes:** `/home/byron/Desktop/AC_1_USA_RE/overlays/mission_overlay_FDAT202_80050000.bin`
  (raw RAM, base `0x80050000`, size `0x90000` = 589824 B, captured in-mission).
- **Language:** `MIPS:LE:32:default` (R3000A, little-endian), image base `0x80050000`.
- **Analysis:** full Ghidra auto-analysis ran clean (585-ish funcs; generic_clib applied).
  Decompiler "Unable to read bytes at 0x8004ad..–0x8004c1.." warnings are EXPECTED:
  the overlay header / jump tables live in `0x8004ADA0–0x8004CFFF`, which is below
  this dump's base and so absent — those are cross-references into the overlay header,
  not errors.

### To open it (does not touch the live session)
Launch a second Ghidra, then File → Open Project →
`/home/byron/Desktop/AC_1_USA_RE/ghidra_mission202/Mission202.gpr`, and open the
`mission_overlay_FDAT202_80050000.bin` program. (If the live GUI holds a global lock,
just start another Ghidra instance; the projects are separate directories.)

### Headless reproduction
```
HL=/snap/ghidra/35/ghidra_12.0_PUBLIC/support/analyzeHeadless
$HL /home/byron/Desktop/AC_1_USA_RE/ghidra_mission202 Mission202 \
  -import .../overlays/mission_overlay_FDAT202_80050000.bin \
  -processor MIPS:LE:32:default -loader BinaryLoader -loader-baseAddr 0x80050000 \
  -scriptPath /home/byron/Desktop/AC_1_USA_RE/ghidra_scripts \
  -postScript AnnotateMission202.java
# then: -process ... -noanalysis -preScript FixVMLabels.java
```
Scripts: `ghidra_scripts/AnnotateMission202.java`, `ghidra_scripts/FixVMLabels.java`.

### Functions / labels annotated (all CONFIRMED via DuckStation RE + this dump)
| addr        | name                        | kind  | note |
|-------------|-----------------------------|-------|------|
| `0x800788C8`| `mission_resource_loader`   | func  | copies+relocates geometry/collision via `FUN_80050FC4` |
| `0x80078A2C`| `mission_enemy_slot_alloc`  | func  | returns free AC-array record (base `0x801A26B8`, stride `0x170`) |
| `0x80078B14`| `mission_spawn_binder`      | func  | `sb 1->[spawnrec+0]` (spawned flag), `sb slot->[spawnrec+1]`, writes record defaults (`+0x65=0xFF`,`+0x1a=4`,`+0x2e=0x7fff`) |
| `0x80078C00`| `mission_spawn_record_init` | func  | record init (within/adjacent to binder body); type ptr `0x8019F590`, AI/collision defaults, back-links to spawn point |
| `0x80078CFC`| `mission_spawn_routine`     | func  | a0=spawn record: bounds-check, `alloc`→`binder`; verified prologue `addiu sp,sp,-0x20` |
| `0x8008B42C`| `mission_script_vm_dispatch`| label | script-VM central dispatch; `lhu` opcode@`s0`, bounds `<32`, `beq`→handlers. Mid-routine target inside the VM func (~`0x8008B41C` / `FUN_8008A0B0`), NOT a func entry |
| `0x8008B830`| `mission_op_spawn_group`    | label | opcode `0x1003` "spawn group N": `s1=0x8019FAB8 + N*44`; `lbu` flag; if `!=1` `jal 0x80078CFC(a0=s1)`. Reached from dispatch `beq @0x8008B45C` |

`0x8008B42C` and `0x8008B830` are deliberately **labels, not functions** — Ghidra
auto-analysis correctly absorbed them into the larger script-VM routine, and forcing
1-byte stub functions there would be wrong. `FixVMLabels.java` removed the stubs and
left primary labels + plate comments.

A program-wide plate comment at `0x80050000` records the swap warning (text identical
to the TL;DR above).

### Level-completion mechanism (added 2026-06-14, live DuckStation RE)

The "mission complete" / level-exit commit path, annotated in the **same Mission202
project**. CONFIRMED live (memory `project_ac1_objective_complete`) and re-verified
against the dump bytes. Programs/blocks noted per symbol.

| addr        | name                         | kind  | program / block        | note |
|-------------|------------------------------|-------|------------------------|------|
| `0x8008A0B0`| `mission_exit_commit`        | func  | Mission202 `ram` block | objective-object update **dispatcher**: indexes mission jump table `0x8004C164` by `(a0-1)` and `jr v0` @`0x8008A104` into per-frame event-case bodies. Runs each frame via `mission_phase_dispatch`. |
| `0x8008A1F4`| `mission_exit_complete_case` | label | Mission202 `ram` block | the EXIT-COMPLETE case body (a `jr v0` target inside `mission_exit_commit`, NOT fallthrough — so the dispatcher's Ghidra function body does not span it). gate `lhu v0,[0x8019F528]`@`0x8008A1EC`; `bnez`@`0x8008A1F4`; else `jal 0x8008A048`/`0x8008A080` (COM "mission complete" sub-handlers) then `jal 0x8004C318`@`0x8008A20C` with `a0=0x80` in delay slot @`0x8008A210`. |
| `0x8008AB68`| `mission_phase_dispatch`     | func  | Mission202 `ram` block | per-frame: `lw [0x8019F51C]` (objective object `0x801C4B40`), `lw 0(v0)`, `jalr v0`@`0x8008AB80` — calls the objective object's update method. |
| `0x8008A8F8`| `mission_eventflags_tick`    | func  | Mission202 `ram` block | per-frame maintainer of event-flags word `0x8019F524`: `lw; and ~0x1000; sw`@`0x8008A938` (clears transient bit `0x1000`). This per-frame write defeats a plain watchpoint on `0x8019F524`. |
| `0x8004C318`| `mission_set_result`         | func  | Mission202 **`ovl202_lower`** block | mission result/end primitive. `sw a0,[0x8019F524]`@`0x8004C324` stores result code (`a0`; `0x80` from exit-complete path); if `[0x8019F528]==0` sets end-countdown `0x8019F528=100` (~debrief in ~100 frames). Called from `mission_exit_commit`@`0x8008A20C`. |

**The lower overlay block.** `0x8004C318` is **below** the main dump base
`0x80050000`. To annotate it in the same project, a new **regular initialized
memory block `ovl202_lower` (`0x8004ADA0`..`0x8004FFFF`, `0x5260` B)** was created
from the first `0x5260` bytes of `disc_map/overlays/ovl202_mission.bin`
(load_base `0x8004ADA0`). Only the lower `0x5260` is loaded — the rest of that
carved image (`0x80050000`+) would overlap the existing `ram` block, so it is
omitted. A `Pcode error at 8004ada8` during disassembly is EXPECTED (`0x8004ADA0`
is the overlay header, not code); `0x8004C318` itself disassembled cleanly
(10 instructions, `jr ra`@`0x8004C338`). The mission VM jump table `0x8004C164`
now resolves to real bytes inside this block.

Resident data addresses referenced in the comments — `0x8019F524` (result/event
word), `0x8019F528` (end-countdown), `0x8019F51C` (objective-object ptr →
`0x801C4B40`), `0x801D0B50` (objectives-complete state, ==3) — live in the
always-resident data region OUTSIDE both blocks, so they are NOT labeled here.

Scripts: `AnnotateMission202Complete.java` (parts A+B), `FixExitCommit.java`
(re-pointed `mission_exit_commit` to the true entry `0x8008A0B0` after the first
pass made a 1-byte stub at the `jr v0` target `0x8008A1F4`), `LabelExitCase.java`
(label+pre-comment on the case body), `VerifyLevelComplete.java` (read-only dump).

### Mission script-VM event switch (added 2026-06-14, static RE + Ghidra)

The dispatcher `mission_exit_commit` @`0x8008A0B0` is the **mission script-VM event
switch**. Bounds check at `0x8008A0DC`: `sltiu v0,a0,10` → **exactly 10 cases**
(`beqz`→default `0x8008A318`). Index = `(a0 & 0xff) - 1`. It loads
`lw v0,[0x8004C164 + index*4]` (`0x8008A0FC`) and `jr v0` @`0x8008A104`.

**Jump table `0x8004C164`** (10 × u32 pointers; lives in the `ovl202_lower` block).
Defined as a Ghidra `JumpTable` override on the `jr v0`, so `mission_exit_commit`
now decompiles as a full 10-case `switch`. Table label = `mission_vm_jumptable`;
each target labeled `mission_vm_caseNN`. Bytes byte-verified against
`disc_map/overlays/ovl202_mission.bin` (table file offset `0x13C4`); entry 10 =
`0x00000000`, confirming the count is exactly 10. **NOTE:** this is a DIFFERENT
dispatch from the secondary spawn-group VM `0x8008B42C`/`0x8008B830` (opcode `0x1003`,
`project_ac1_dynamic_spawn`) — do not conflate; that one is a `lhu`-opcode `<32`
beq-chain, this one is the `(a0-1)`-indexed objective-event table.

| case | a0 | target addr  | label              | meaning (CONFIRMED by decompile unless "hyp") |
|-----:|---:|--------------|--------------------|-----------------------------------------------|
| 00   | 1  | `0x8008A10C` | `mission_vm_case00`| `jal 0x8008A048` (COM/broadcast, a0=s3); loops world-object array `&DAT_801D0BA0` (stride 0x40) clearing byte `+6`. Reset/broadcast op (hyp meaning). |
| 01   | 2  | `0x8008A158` | `mission_vm_case01`| `jal 0x8008C2FC(a0=s0)` — secondary VM/COM routine (hyp). |
| 02   | 3  | `0x8008A168` | `mission_vm_case02`| `jal 0x8008A048(a0=s0)` — COM/broadcast sub-handler (hyp). |
| 03   | 4  | `0x8008A178` | `mission_vm_case03`| **SET-TIMER** (old cmd4): `jal 0x8008A048`; `jal 0x8008A778(timer=operand*0x16, mode, a2)`; `sh result,[0x8019F52C]` (displayed timer). Mode s4: 1→0x200, 2→0x80, else→0x100. |
| 04   | 5  | `0x8008A1E8` | `mission_vm_case04`| **EXIT-COMPLETE / SUCCESS**: gate `lhu [0x8019F528]`; if 0 → `jal 0x8008A048/0x8008A080` then `mission_set_result(0x80)`. (= `mission_exit_complete_case` label is at `0x8008A1F4` inside this body, the gate `bnez`.) |
| 05   | 6  | `0x8008A21C` | `mission_vm_case05`| **RESULT(0x100) via COM**: gate `lhu [0x8019F528]`; if 0 → `jal 0x8008A048/0x8008A080`, fall into case-07 tail `0x8008A274` → `mission_set_result(0x100)`. |
| 06   | 7  | `0x8008A248` | `mission_vm_case06`| `jal 0x8008A048/0x8008A080` (COM broadcast), then return. Pure broadcast op. |
| 07   | 8  | `0x8008A260` | `mission_vm_case07`| **SUCCESS-IF-99** (old cmd8): `FUN_80052A2C(99)`; if ret==1 → `mission_set_result(0x100)` @`0x8008A274`. |
| 08   | 9  | `0x8008A284` | `mission_vm_case08`| **OBJECTIVE-OBJ METHOD** (old cmd9): `lw [0x8019F51C]`(obj `0x801C4B40`); `(*(obj+8))(a0=s1)`. |
| 09   | 10 | `0x8008A2A8` | `mission_vm_case09`| `jal 0x8008A048`; loops world-object array `0x801D0B68` (stride 0x40): for each `obj!=0`, call `(*(obj+4))(obj, mode|0x8200, 0)`. Broadcast to world objects (hyp meaning). |

Result codes committed via `mission_set_result` (`0x8004C318`): `0x80` (case04),
`0x100` (cases 05 & 07). These match the live `0x8019F524` event-flags semantics
documented in `project_ac1_objective_complete`.

Script: `DefineMissionVMTable.java` (defines the 10 pointers, labels table+cases,
writes the JumpTable override; idempotent — verifies each entry against the carved
disc bytes before labeling). The switch decompiles cleanly and persists across
reopen (verified read-only).

## Cross-overlay references (do not chase blindly)
The mission VM jump table is at `0x8004C164` and the overlay-header pointers at
`0x8004ADA0+`. Those addresses are **below** this dump's base, so Ghidra shows them as
unreadable. To resolve them, use the separately-carved overlay images
(`disc_map/overlays/ovl202_*.bin`, base `0x8004ADA0`) referenced in
`project_ac1_mxt_loader`, or extend this dump downward.

## Files
- Doc (this): `/home/byron/Desktop/AC_1_USA_RE/overlays/OVERLAY_MAP.md`
- Dump: `/home/byron/Desktop/AC_1_USA_RE/overlays/mission_overlay_FDAT202_80050000.bin`
- Provenance: `/home/byron/Desktop/AC_1_USA_RE/overlays/README_mission_overlay.md`
- New Ghidra project: `/home/byron/Desktop/AC_1_USA_RE/ghidra_mission202/Mission202.gpr`
- Annotate scripts: `/home/byron/Desktop/AC_1_USA_RE/ghidra_scripts/{AnnotateMission202,FixVMLabels}.java`
- Level-complete annotate scripts: `/home/byron/Desktop/AC_1_USA_RE/ghidra_scripts/{AnnotateMission202Complete,FixExitCommit,LabelExitCase,VerifyLevelComplete}.java`
- VM-switch table script: `/home/byron/Desktop/AC_1_USA_RE/ghidra_scripts/DefineMissionVMTable.java`
- Pre-switch-table Mission202 backup: `/home/byron/Desktop/AC_1_USA_RE/_ghidra_mission202_backup_pre_switchtable_2026-06-14/`
- Pre-task Ghidra backup: `/home/byron/Desktop/AC_1_USA_RE/_ghidra_backup_2026-06-14/`
- Pre-level-complete Mission202 backup: `/home/byron/Desktop/AC_1_USA_RE/_ghidra_mission202_backup_pre_levelcomplete_2026-06-14/`
