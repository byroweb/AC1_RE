# AC1 Player Interactions — Circle-context (door / item), COM dialogs, SFX

Target **SLUS-01323 (v1.1)**. How the in-mission **Circle button** acts as a
context-sensitive "interact" (open a door, pick up a weapon) instead of the laser
blade, plus the **COM message dialog** and **SsVm sound** systems those
interactions drive. Companion to `docs/MISSION_SYSTEM.md`, `docs/OBJECT_STATS.md`
(world-structure objects) and `overlays/OVERLAY_MAP.md` (overlay provenance).

Notation: `CONFIRMED` = live-verified in DuckStation (watchpoint / diff) and/or
disasm-verified. `HYPOTHESIS` = inferred, not fully ground-truthed.

> All addresses are runtime RAM for the NA v1.1 release. Handlers in the
> `0x801Cxxxx` range live in the **per-mission objective overlay** (FDAT `2N`,
> loaded to `0x801C4B40`); the SsVm sound engine is in the **resident EXE**.

---

## 0. The context-sensitive Circle button  CONFIRMED

Circle is the **laser-blade** attack by default (a blade swing **lunges the mech
forward** and sets weapon-fire flag `0x80007264 = 1`). When the player AC is in
range of an **interactable** (a door or a pickup item), the same Circle press is
re-routed to that object's **interact handler** instead — **no lunge, no
`0x80007264`**. That blade-vs-interact signature is the reliable way to tell, in
a memory diff, whether a Circle press was a swing or an interaction.

The interactables themselves are **world-structure objects** — the same array
family as the destructible generators (`docs/OBJECT_STATS.md`): base
`0x801D0B68`, stride `0x40`, handled by the objective overlay.

---

## 1. Sliding door  CONFIRMED

Captured live: save-state at the door, one Circle press → door slides left over
~3 s and stays open (auto-closes only when the player backs away). Method:
idle-stable region diff of `0x801B0000–0x801E0000` (0 idle churn → 29 clean
changes) + a write-watchpoint.

**The door object** is a world-structure record in the `0x801D0B68` array
(captured instance base `0x801D2B28`).

| field | meaning |
| --- | --- |
| record `+0x30` (state word) | **bit `0x100` = "closed"**. Opening = `state &= ~0x100`. Other bits = flags/index. |
| geometry sub-record (`~0x801D32E8`) | panel coords, updated each frame while sliding |
| movement pair (`0x801DBD44` / `0x801DBD90`) | mirrored slide offset/velocity (`0→0x3203`) |

**Open path** (objective overlay, reached from the object's per-frame
think/interact dispatch when Circle is pressed in range):

```
; caller @0x801C6CA4
a2 = 0x801D0B68 + doorIndex*0x40     ; the door record
v0 = [a2+0x30]                       ; state word
v0 = v0 & ~0x100                     ; clear the "closed" bit
jal  0x801C6B68                      ; open routine: substate=4 ("opening") +
sw  v0,[a2+0x30]                     ;   drive panel-slide geometry
```

- **`0x801C6B68`** = open routine. `a0` = door geometry sub-record; sets door
  **substate = 4** ("opening") and animates the slide.
- This is the same handler family as the generator/destructible on-hit handler
  `0x801C58C4` — all objective-object code.

**Watchpoint gotcha (CONFIRMED):** the door flag is written by a **word/halfword
store to the aligned address `0x801D2B58`**, so a byte-watchpoint on `+1`
(`0x801D2B59`) silently **misses** it. Always watch the aligned word.

---

## 2. Item pickup — "AC weapon obtained"  CONFIRMED (behavior) / HYPOTHESIS (storage)

Captured live: save-state on a floor item (too low to show in the 3rd-person
camera), one Circle press → a **non-pausing translucent dialog** reading
**`COM : AC weapon obtained`** appears. No blade lunge, no `0x80007264`.

**Footprint of the pickup (CONFIRMED):**
1. an **SsVm sound** (see §4);
2. the **COM dialog** built into the display lists (`0x80150000–0x80152000` +
   `0x8014Cxxx`) — transient while the dialog is shown;
3. a generic action flag `0x80031B22 = 1` (also set by a blade swing).

**No persistent "item collected" flag was found in main game-state RAM**
(exhaustively diffed with idle-baseline subtraction across `0x80000000–0x80100000`,
`0x80100000–0x80140000`, `0x801B0000–0x801E0000`). HYPOTHESIS: the obtained
weapon is queued through the COM-message system and committed to the **owned-parts
memory-card save at mission end**, not held as a live mid-mission RAM byte. To
pin the handler, breakpoint the COM-dialog/message writer (what fills the
`0x80150000` dialog text), not a memory diff.

---

## 3. COM message dialog system  CONFIRMED (render) / HYPOTHESIS (queue)

The non-pausing translucent "`COM : …`" boxes (e.g. *AC weapon obtained*, the
*last-objective* notice — see `docs/MISSION_SYSTEM.md`) are **readouts**, not
state. They are drawn as **display-list primitives** into `0x80150000–0x80152000`
and `0x8014Cxxx–0x8014Exxx` (a large contiguous `0→nonzero` block of stride-~0x34
coord/colour records), and persist for a timed window without pausing the sim.
The COM text/event is queued by gameplay code (the door/item/objective handlers);
the exact queue structure is not yet RE'd.

---

## 4. SsVm sound engine (resident EXE)  CONFIRMED — landmark

AC1 uses the PsyQ **SsVm** (Sound Sequencer / Voice Manager) library. Found while
watchpointing what looked like a pickup flag — it was the pickup **SFX**.

- **`UT_KEYV_OBJ_180` @ `0x80021880`** — sound-trigger: builds a voice/note entry
  then keys it on (`_SsVmDoAllocate`, `vmNoiseOn`, `note2pitch2`, `_SsVmKeyOnNow`).
- **Voice/note table `0x80040938`**, stride `0x1A`.
- **Sound state scratch:** `0x80040D..` (e.g. `0x80040DE4` set on key-on),
  `0x80042B50..0x80042B6A`, `0x80039ABE..0x80039B1A` (SPU pitch/vol).

**RE caution (important for memory diffs):** pressing Circle fires a sound on
**any** action (blade OR interact), so `0x80040Dxx` / `0x80042Bxx` / `0x80039Axx`
light up on every press and are **red herrings** for the gameplay effect. Exclude
them, along with render/OT lists (`0x8000857C–E647`, `0x80090xxx`, `0x800B12xx`,
`0x8014Cxxx–0x8014Exxx`), animated-object data (`0x801A6A48–6E4C`), and the stack
(`0x801FFDxx`).

---

## 5. Capture method (reusable)

These were isolated with **idle-baseline subtraction** — see the `mem-diff-baseline`
skill for the full toolkit:

1. From a save state (deterministic replay), snapshot an **idle-stable** region
   and confirm 0 churn (world-object band `0x801B0000–0x801E0000` is stable;
   render/animation bands are not).
2. Perform the action, diff → the stable-region changes are pure signal.
3. Reload, set a **write-watchpoint on the aligned word** of the found flag,
   redo → `pc`/`ra` at the hit give the handler + caller.

For a flag a driver rewrites **every frame** (so a watchpoint trips uselessly),
**NOP that driver's store** (`write_memory <pc> 00000000`), catch the real setter
on the now-clean watchpoint, then restore the bytes (DuckStation patches PS1
overlay code with no I-cache trouble; a save-state reload also restores it).

---

## 6. Ghidra

Door/item handlers live in the **objective overlay** (`0x801Cxxxx`, FDAT `2N`),
which swaps at the same addresses as other overlays — annotate them only against
the correct overlay image, never the resident program (see
`overlays/OVERLAY_MAP.md`). The SsVm functions are **resident** and match the
base program directly.
