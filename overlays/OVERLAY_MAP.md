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
- Pre-task Ghidra backup: `/home/byron/Desktop/AC_1_USA_RE/_ghidra_backup_2026-06-14/`
