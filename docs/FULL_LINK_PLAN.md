# Full-link build and byte-provenance audit — scoping plan

Status (2026-10-22): **P0 done**; see [FULL_LINK_P0.md](FULL_LINK_P0.md). P0
changed three things:
- D4 (bss order) is now the main risk: about 1,100 bss symbols in the program
  overlays, and the mission buffer is itself overlay bss.
- D5 (fixed RAM) is empty.
- D6 gains an exe ↔ ovl201 link cycle and a 64th unit (`fdat188`).

P1 is next.

## Goal

Today every match is checked one function at a time (`tools/match/match_fn.py`
against the retail bytes, re-verified by `re/match/check_all.sh`). That supports
"100 % of functions match". It does not support "**the game rebuilds from this
source**", which is the claim a skeptic will test.

**Definition of done:**

1. `make` builds every code unit from source, then `make verify` prints a SHA-1
   match for each against `overlays/CODE_UNITS.tsv` (`sha1_v1_0`, the same bytes
   as v1.1):
   - the resident exe (`SLUS_013.23`, code-identical to `SCUS_941.82`);
   - the 4 program overlays (FDAT entries 201–204);
   - the 57 distinct mission overlays (plus `fdat168`, an alias of `fdat166`).
2. A **byte-provenance audit** accounts for every output byte by origin. It fails
   the build on any forbidden origin.
3. A **no-disc build** links the code-and-data part of every unit and matches
   published per-section hashes, with no disc image present.
4. A **shift build** moves every unit's contents, and the game still boots and
   plays.

Out of scope: rebuilding `FDAT.T`, other `.T` archives, or the disc image.
Assets stay extracted from the verifier's own disc (`tools/extract`,
`tools/disc`).

## Where we stand (facts the plan rests on)

| fact | source |
|---|---|
| 63 code units: exe at `0x80011000`; 201–204 all at `0x8004ADA0`; mission groups at `0x801C4B40` (47 units), `0x801C8460` (5), `0x801B80C8` (4) | `overlays/CODE_UNITS.tsv` |
| Section order in every unit is **rodata → text → data**, packed at 4 bytes, with no padding between objects. Program overlays start with an `"ENERGY"` header; mission units start with a 0x120-byte slot/pointer-table header | `CODE_UNITS.tsv`, `docs/MXT_LOADER.md`, `docs/MISSION_OVERLAY_CODE.md` |
| Each overlay image is zero-padded to a 0x800 multiple and ends in a checksum word (seed `0x12345678`); `re/src/ac1_mxt.c` already computes it | spot check of all 61 images; `docs/MXT_LOADER.md` §5 |
| Bytes by section, all units: rodata ≈ 105 KB, text ≈ 2.36 MB, data ≈ 438 KB | `CODE_UNITS.tsv` |
| 6,334 functions, of which 563 are PsyQ 3.7 library code (all in the exe: `0x80016EA4–0x80030368`, `AddDrv`/`DelDrv`, `2MBYTE.OBJ` start-up), ≈ 108 KB | `tools/loop/functions.tsv`, `docs/REFERENCE.md` |
| 5,541 manifest rows (twins included); about 229 game functions still open | `re/match/manifest.tsv`; golem bake-off report |
| Matched C names its data by **absolute address**: hundreds of `--defsym D_xxxxxxxx=0x…` arguments, and no data or bss is defined in C yet | `manifest.tsv` |
| The only inline asm in matched C is the PsyQ-style GTE macros (`include/ac1/gte.h`). They emit COP2 words with `.word 0x4A……`/`0x4B……` | grep of `re/match/**/*.c` |
| Object (TU) boundaries **cannot be seen** in the bytes. The 194-TU map is a modularity hypothesis | `docs/CODE_ARCHAEOLOGY.md` §1, `tools/archaeology/tu_map.tsv` |
| Mission units call their host overlay by absolute `jal`: the `0x801C4B40` group calls ovl202 (and the exe), `0x801C8460` calls ovl204, `0x801B80C8` calls ovl203 | `docs/MISSION_OVERLAY_CODE.md` |

## Design decisions (recommended)

### D1 — Toolchain: matcher compile path + GNU ld + generated linker scripts

- **Compile** exactly as `match_fn.py` already does: host `cpp` → GCC 2.7.2 `cc1`
  → maspsx 2.56 → mipsel `as`, keeping the `.align 3 → .align 2` rodata
  rewrite. One flag set, one source of truth. Factor the stage commands out of
  `match_fn.py` instead of copying them.
- **Link** with mipsel GNU `ld`, one generated script per unit. The script uses
  `.rodata` then `.text` then `.data` then `.bss`, 4-byte input alignment, and
  `KEEP` in manifest order. psylink isn't available and isn't needed: it packed
  4-aligned with no padding, which `ld` can reproduce.
- **Post-link:**
  - exe: an `objcopy -O binary` image wrapped in a generated 0x800-byte PS-X EXE
    header (pc0, t_addr, t_size, stack, region string).
  - overlays: binary image → zero-pad to 0x800 → checksum word from
    `ac1_mxt_fix_checksum`.
- **Splitter:** generate `splat` YAML per unit from `CODE_UNITS.tsv`,
  `functions.tsv` and `tu_map.tsv`. splat is the community-standard tool, which
  helps credibility, and Wyrelade's per-unit YAML is a cross-check. splat only
  supplies the scaffolding (`INCLUDE_ASM` stubs, data/rodata `.s` files); the
  matched C stays where it is.

### D2 — Granularity: start one-function-per-object, let the link prove the TU map

Text bytes don't depend on object boundaries, but rodata and data order do. GCC
2.7.2 writes each function's strings and jump tables into `.rdata` as it emits
the function, and file-scope tables come first.

- Start with **one object per function in address order**, which is what the
  manifest already is.
- Wherever retail rodata or data order disagrees, merge functions into a TU (and
  move file-scope tables to the top). Statics shared by several functions force
  the same merge.
- The resulting object list **confirms or corrects** the `tu_map.tsv`
  hypothesis. That's a deliverable for `docs/CODE_ARCHAEOLOGY.md`, not just a
  cost.
- Twins: each mission unit is its own link, so the shared "frozen object
  library" (27 units) becomes **one shared source** linked into each unit. That
  is probably how the original was built, and it removes most of the twin
  bookkeeping.

### D3 — Data: three tiers, and the audit shows which tier each byte is in

| tier | form | shiftable? | counts as source? |
|---|---|---|---|
| T0 | raw slice of the retail unit, read from the user's disc at build time | no | **no** (red in the audit) |
| T1 | splat-symbolised `.s` data: `.word func_…`, `.word D_…`, literal words otherwise | yes | partially (amber) |
| T2 | C definitions (typed structs, tables, strings) | yes | yes (green) |

- The exe and overlays first reach a byte match at **T0**: a "byte-identical
  shell" that tests the linker, header and checksum pipeline before any data
  work.
- Promotion to T1 is mechanical (splat plus symbol tables). T1 → T2 is the long
  tail, done per subsystem alongside the `docs/` write-ups.
- **Done** means no T0 bytes in any code unit, and T1 only where a table is
  better kept as data (see decision Q2).

### D4 — bss / COMMON allocation

The code is compiled `-fcommon -G0`, so uninitialised globals are COMMON symbols,
and psylink chose their addresses. Every `D_` reference into bss is baked into
code bytes, so the order has to be reproduced.

1. Infer psylink's COMMON ordering rule empirically from the retail addresses (by
   object order, first reference, name or size). Encode it in a per-unit
   `bss.c` compiled `-fno-common` with the symbols defined in that order.
2. Fallback: pin the stragglers in the linker script. Each pinned symbol is
   flagged in the audit because it isn't shiftable.

### D5 — Fixed RAM addresses outside every unit

Some `D_` addresses fall outside every unit's image and bss, for example the
mission-object pointers around `0x8019F51C`. Those were very likely fixed
addresses in the original too, written as `#define` casts.

- They go into a shared `ram.ld` as absolute symbols, or into headers as macros.
- The audit separates them from pinned symbols **inside** a unit's own range.
  Only the latter break shiftability.

### D6 — Link graph for overlays

1. Link the exe first and export its symbols (`ld --just-symbols`).
2. Link each program overlay 201–204 against the exe symbols.
3. Link each mission unit against the exe symbols plus its host's (ovl202, 204 or
   203, per the table above).
4. Generate the headers (`"ENERGY"` block; the 0x120-byte mission slot tables) as
   C structs of function pointers, so the linker resolves them.
5. Build `fdat168` from the same objects as `fdat166` and check its hash
   separately.

### D7 — PsyQ libraries (needs your decision; Q1 below)

The 563 library functions (~108 KB, ~4 % of all code bytes) aren't game code.
The choices:

| option | how | verifier needs | audit label |
|---|---|---|---|
| **A** (recommended to start) | Link the official PsyQ 3.7 `.LIB` files (`libsnd` 3.5, `CARD.OBJ` 3.0), converted to ELF with a psyq-obj-parser-style tool | their own copy of the SDK | SDK |
| B | Take the library block's bytes from the user's own disc exe | their disc | SDK-from-disc (red under the strict bar) |
| C | Matched library sources: C where Sony wrote C, `.s` where the original module was hand-written asm (libapi stubs, parts of libgte/libetc). Reuse public PSX decomp work where the library versions agree | nothing extra | green / original-asm |

**Recommendation:** use **A or B** to reach the first full exe hash, and present
the public claim as "100 % of the **game code**" with SDK bytes counted on their
own line. Do **C** only if you want the stronger "whole executable" claim. It's
a separate project of roughly the size of one large program overlay, and some
of it is legitimately asm.

## Byte-provenance audit (`tools/provenance/`)

**Inputs:**
- the `ld` map file and per-object symbol tables;
- `manifest.tsv` and the splat config;
- a list of every file the build read from the disc, with SHA-1s (the build logs
  this itself).

**Classes**, one per output byte:

| class | meaning | under the strict bar |
|---|---|---|
| `c-text` | code compiled from C | ok |
| `c-data` / `c-rodata` | data defined in C, including compiler-emitted strings, jump tables and constants | ok |
| `gte-inline` | COP2 words from the whitelisted `gte.h` macros (all inside `c-text`) | ok |
| `asm-data` | T1 symbolised data | ok, reported |
| `asm-stub` | `INCLUDE_ASM` placeholder for a still-unmatched function | must reach 0 |
| `retail-slice` | T0 or any `.incbin` of retail bytes in a code unit | **fail** |
| `sdk` | PsyQ library code/data, by option A/B/C | reported on its own line |
| `generated` | exe header, overlay checksum, zero padding | ok |

**Hard failures:**
- any `retail-slice` byte;
- any `.word` inside `.text` that isn't a whitelisted COP2 encoding;
- any linker-script absolute symbol that lands inside the unit's own image or bss
  range;
- any byte not covered by exactly one class.

**Outputs:**
- a per-unit table and grand totals (Markdown, for the README);
- a JSON treemap for a `browser-tools/provenance-map` page: every unit as a
  rectangle, coloured by class, with a click-through to the source file. This
  fits the existing match-viewer style.

**Companion checks** (CI jobs, not part of the audit script):
- **No-disc build:** run with the disc path unset. Every code unit links, with
  zeros where asset bytes would go, and the build checks published per-section
  hashes for text, rodata and data.
- **Shift build:** add `-DAC1_SHIFT` to insert a 16-byte dummy function at the
  head of each unit's text and an 8-byte dummy at the head of data. Boot in
  DuckStation (the MCP fork already scripts this) and confirm three things by
  screenshot or RAM watch: the title screen, the garage, and that mission 1
  starts.
- **Mutation demo:** change one documented constant (for example from
  `docs/COMBAT_PHYSICS.md`) and show the hash change and the in-game effect.
- **Reproducible container:** a Dockerfile that builds GCC 2.7.2 from the public
  source with `tools/match/patches/gcc-const-double-slots.patch`, plus maspsx
  and binutils, all pinned by hash. CI runs the build with networking off.

## Phases

Sizes are relative (S < M < L < XL). The matching backlog runs in parallel and
isn't counted here.

| # | phase | exit criterion | size |
|---|---|---|---|
| P0 | **Readiness audit.** (a) Run `drive.py verify` and confirm zero masked matches. (b) Sort every `--defsym D_…` into own-rodata / own-data / own-bss / other-unit / fixed-RAM. (c) Look for any `D_` landing mid-object, which signals a wrong function size. (d) Decide Q1–Q3 | a report table; decisions recorded | S |
| P1 | **Exe shell.** Splitter config, linker script, PS-X EXE header generator, `make verify`, with everything at T0 / `INCLUDE_ASM` | exe SHA-1 matches with 0 % C | M |
| P2 | **Exe with source.** Swap in the 125 matched game functions as one object per function, merging TUs where rodata order demands; libraries per D7 | exe SHA-1 matches; audit runs | M |
| P3 | **Program overlays 201–204.** Header struct, `--just-symbols` link against the exe, checksum post-step; 201 first (1,091 functions, the largest) | 4 SHA-1 matches | L |
| P4 | **Mission overlays.** Template-generate the 57 configs; one shared library-family source; the 0x120 header as C | 58 SHA-1 matches (alias included) | M |
| P5 | **Data and bss.** T0 → T1 everywhere (mechanical); infer the COMMON order; then T1 → T2 by subsystem | zero `retail-slice`; pinned-in-range count → 0 | XL (the long tail) |
| P6 | **Proof kit.** Audit failure rules on in CI, the provenance treemap page, the no-disc and shift jobs, the container, and a `verify.sh <your-disc>` for outsiders | all CI green | M |
| P7 | *(optional)* **Matched PsyQ libraries** (D7 option C) | `sdk` bytes reclassified | L |

P1–P4 can start now: every unmatched function is just an `INCLUDE_ASM` stub, so
the full link doesn't wait for 100 %. As matching finishes, the `asm-stub`
count in the audit becomes the progress number.

## Risks and unknowns

- **psylink COMMON order** (D4) may not follow any simple rule. Mitigation:
  pinning, which is visible in the audit, is the fallback, so it can't block
  P1–P4.
- **Wrong function boundaries.** Per-function matching tolerates a function that
  swallowed its neighbour's tail padding or a data island in `.text`; a full link
  doesn't. P0(c) and the first links will expose these.
- **Rodata ordering inside TUs** may not match per-function emission order when
  a TU has file-scope `const` tables or literals shared across functions. The
  fix is merging into TUs (D2): more work, but it doesn't block anything.
- **Twins that diverged only in relocations** may need per-unit `#if` in the
  shared library source, as in the 202/203/204 build-configuration splits from
  `CODE_ARCHAEOLOGY.md` §4.
- **Embedded asset-like tables** inside overlay `.data` (meshes, palettes,
  mission tables) make up much of the 264 KB of mission data. Moving them all to
  T2 C may not be worth it; see Q2.
- **Exe header fields** (stack, region string) must be reproduced exactly. Low
  risk: they're generated, and the generator is small.

## Decisions

Decided 2026-10-22:
- **Q1:** option **A**. The first exe hash links your own PsyQ 3.7 `.LIB`s; C
  stays optional (P7).
- **Q2:** large asset-like tables may stay **T1** symbolised data. Nothing stays
  T0.
- **Q3:** the matched C moves once, in P2, into one `src/` tree that both the
  full link and `match_fn.py` read.

The original questions, kept for the record:

- **Q1 — PsyQ libraries:** option A (your SDK `.LIB`s), B (bytes from your disc)
  or C (matched sources), and which one to use for the first hash. Recommended:
  A or B now, C later if you want the whole-exe claim.
- **Q2 — Data bar:** must all overlay `.data` reach T2 C? Or may large
  asset-like tables stay T1 symbolised data, or be classed as assets extracted
  from the disc? Recommended: T1 is acceptable for asset-like tables, but nothing
  stays T0.
- **Q3 — Layout:** a new top-level `src/` + `asm/` + `Makefile` tree, with
  `re/match/` as the matched-C store it pulls from? Or move the matched C into
  `src/` and have `match_fn.py` read it there? Recommended: move once in P2, so
  the full link and the oracle share one copy.
