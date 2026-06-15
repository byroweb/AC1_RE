# Sub-trace B — CORRECTION/RETRACTION (2026-06-11)

**The earlier claim in this file — "missions reuse stage PA00, differing only in
sub-resource bytes" — is WRONG. Retracted.**

## What disproved it
User ground-truth: Eliminate Squatters (high-ceilinged room), Eliminate Strikers
(bridge over water), Reclaim Oil Facility (oil facility), and garage "Test AC" (arena)
are visibly DIFFERENT environments — they cannot all be PA00.

Live test: armed a **write-watch on the stage byte `0x8004121B`** AND a read-watch on
`0x8004121D`, then played the **entire** Reclaim Oil Facility mission.
- **Neither watch fired the whole mission.**
- Path buffer `0x8008D928` stayed `"P0\PA00.T"`; byte `0x8004121B` stayed `0x00`.
- The GameState descriptor bytes were **byte-identical** across Strikers / Test AC /
  Reclaim Oil Facility (`00 00 00 03 01 01 00 0c ff ff ff 03 00 8a 02 28`).

Three different environments → identical bytes → those bytes are NOT per-mission.

## Corrected understanding
`FUN @ 0x8004F1A8` (reads `0x8004121B`, builds `"P{n}\PA{nn}.T"` at `0x8008D928`, loads
slot 0) is a **bootstrap / common loader**: it loads PA00 once (briefing / garage /
common assets), with the byte defaulting to 0. **It is NOT the mission stage loader.**
The real walkable stage (oil facility / bridge / room) loads through DIFFERENT code that
has not yet been found — it does not touch `0x8004121B` or `0x8004121D`.

### What PA files actually are (revised hypothesis)
The doc/REFERENCE label "PA##.T = stage/map geometry packs" looks WRONG. Evidence:
- The companion AC1mod viewer's PA viewer (separate repo) shows **mission-assignment-screen
  imagery** inside PA files (user observation).
- The renders decoded PA00 entries as **AC/mech parts, small props, and effects/sprites**
  (e21 = an AC core piece; e137 = a prop; e125-165 = effects) — object geometry, not a
  level hull.
- PA00 is loaded as a **common bundle at every mission start** (the bootstrap above).
So PA files appear to be **object / AC / MT / effect + assignment-screen asset packs**,
NOT the playable environment. The walkable stage geometry is a separate, still-unknown
subsystem (different files and/or loader).

## Impact on the other Phase-1 items
- **A (gouraud 0x34/0x3c)** — UNAFFECTED. Pure geometry-decode, validated on the raw PA
  files (0 OOR / 72 files). Still correct.
- **E (effect slots e125-165)** — UNAFFECTED. Format-level decode of those entries.
- **C (sub-resource loader, 0x8004F2xx)** — the CODE trace is accurate, but its CONTEXT
  was mislabeled: it loads sub-resources into the **bootstrap/common (PA00) context**,
  not per-mission stage assembly. The addend→slot-category mapping is real behavior;
  "how stages are assembled" is NOT what it shows. See caveat in [subresource_loader_C.md](subresource_loader_C.md).
- **D (block table 0x8019F538 binding)** — the runtime MECHANISM (instance[+0x0a] →
  block record → +0x28 geometry ptr) is real and was observed live, but only in the
  **training mission**, which is itself a PA00/common context. Whether real mission
  stages use the same path with other PA files is UNPROVEN.

## Correct way to find the real stage loader (open)
The `.T` container loader is `FUN_800165E4` (MXT loader; called at 0x8004F244 for the
bootstrap). EVERY `.T` load goes through it. Plan: arm an **execute breakpoint on
`0x800165E4` BEFORE entering a real stage**, then drop into the mission; log each call's
args/path. The stage PA (a `PA##.T` other than PA00) will appear, revealing the real
stage file and — by backtracking the caller — the real stage selector. Alternatively,
dump the loaded stage geometry work-RAM while standing in a real stage and match it
against the 72 PA files to identify the stage file empirically.

(Lesson: validate load-pipeline claims against the actually-rendered environment /
actually-loaded filename, never against a doc-supplied address alone.)
