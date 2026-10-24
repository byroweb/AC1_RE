# tools/loop — ranking and bookkeeping for the matching-decompilation loop

Three small Python tools that decide *which* function to try next and record
*what happened*.  They do not compile or compare anything: the only judge of a
match is the oracle, `tools/match/match_fn.py` (final line `MATCH ...` or
`MISMATCH ...`, exit status 0 only on MATCH).

```
inventory.py  -> functions.tsv   every function in the exe + overlays, with difficulty features
rank.py       -> stdout          unmatched, non-PsyQ functions, easiest first
ledger.py     -> ledger.tsv      append-only outcomes: match / give-up per function
units.py      -> stdout          unit name -> image file, load address, match_fn.py flags
drive.py      -> stdout, re/match/<unit>/func_<ADDR>.c
                                 the driver: next work package / try the oracle / give up / status / verify
make_queues.py                   rebuilds queue-A.tsv / queue-B.tsv from rank.py;
                                 --bodies: credit-per-effort queue-S1..S6.tsv over twin groups
merge_manifest.py                dedupes + sorts re/match/manifest.tsv after a merge
twins.py      -> twins.tsv, twins_programs.tsv, re/match/twin_replay.tsv
                                 clones (mission overlays; exe + ovl201..204): group by masked body,
                                 replay a matched C file on its twins
progress.py   -> stdout          matched / total per component, Wyrelade-README table shape (--compare)
sweep.py      -> re/match/<unit>/func_<ADDR>.c
                                 zero-agent sweep: m2c drafts straight into the oracle, no LLM
permute.py    -> re/match/<unit>/func_<ADDR>.c
                                 permuter tier: levers.py rewrites + decomp-permuter on near misses, no LLM
memgate.py    (library + CLI)    memory gate: quote a job's commit, admit it only above a headroom,
                                 stop the newest job if free commit runs low (used by permute.py run)
doctor.py     -> stdout, re/match  one front door: classify a near miss from one compile, run the tool that fits
regtie.py     -> re/match          register-tie tier: read cc1's allocation dumps, steer the deciding tie
structdiff.py -> stdout, re/match  where the shape differs, in C lines (object line table), + structural moves
nearport.py   -> stdout, OUTDIR    near-copies twins.py misses: families of open bodies, auto-port from a matched copy
m2cfix.py     (library)          sweep.py's draft fix-ups: stack locals, frame union, casts, cc1-driven repair
golem.py      -> re/match/<unit>/func_<ADDR>.c
                                 local-LLM tier: compile fixes and near-miss edits from a model in LM Studio
overnight.sh  -> re/match/<unit>/func_<ADDR>.c
                                 unattended machine pass: queue, sweep, two-pass permuter, replay, verify, commit
router.py     -> router_labels.tsv, router_features.tsv, queue-router.tsv
                                 tier router: which tier closed each match (labels), static + draft features,
                                 held-out eval by body, and the tier to try next per open function (rank)
```

### Overnight machine pass (2026-10-21)

```bash
bash tools/loop/overnight.sh     # from the repo root; a few hours at the default -j 4, no LLM
```

It runs one branch off origin/main and a queue of every open twin group, programs then
missions (`make_queues.py --bodies -N 1`, with `twins_programs.tsv` and `twins.tsv`), in
this order:
1. `sweep.py` with only `ctx-spframe,ctx-fix-frame,neighbour`. These are the drafts that
   new shared headers and new matched near-copies can change, and they gave nearly every
   sweep match in the 2026-10-20 runs.
2. `permute.py` with `--max-ratio 0.2` (the lanes already ground everything within 0.1) for
   `QUICK`=120 s per candidate, then `DEEP`=300 s for up to `DEEP_N`=50 within
   `DEEP_WORDS`=12 differing words, each starting from its lever text. On the first night
   every deep-pass gain landed inside 300 s, and the candidates further out stayed hundreds
   of permuter points off.
   The 1-3 word near misses go to `$WORK/nearmiss.tsv`: they are the cheapest hand matches.
3. `twins.py replay --scope all` and `drive.py verify`.
4. A commit when verify says all matched. Steps 3-4 are skipped when no manifest row was
   added: they take ~8 min even with nothing new.

Matches are written as they land, so an interrupted run keeps them. The knobs are
environment variables (top of the script); `LIMIT=8 QUICK=20 DEEP=40 DEEP_N=2 BRANCH=0
COMMIT=0` is a quick smoke test.

Watch it with `python browser-tools/match-viewer/viewer.py` (below: the stage, the active tool, each
body's asm diff, and a place to leave hints that steer the next pass), or replay a
finished work dir with `--replay`.

### Program sprint in four commands (2026-10-19)

```bash
python tools/loop/twins.py group                        # twins.tsv + twins_programs.tsv (~2 s)
python tools/loop/make_queues.py --bodies --out-dir DIR # 6 disjoint body queues, best credit first
python tools/loop/sweep.py --queue DIR/queue-S1.tsv ... # machine drafts first; replays twins at the end
python tools/loop/drive.py verify -j 24 -q              # every manifest row, ~80 s instead of ~12 min
python tools/loop/progress.py --compare                 # where we stand vs. Wyrelade's README
```

Then matchers take `drive.py next --queue tools/loop/queue-Sk.tsv` (rows the sweep
closed are skipped) and `twins.py replay --scope programs` after each match.  The
bodies of 1 KB and up are in `queue-L1.tsv`, closest draft first (`make_queues.py
--bodies --closeness tools/loop/closeness.tsv --min-size 1024 -N 1 --prefix queue-L`).

`drive.py` is the only one that compiles anything, and it does so by calling
`tools/match/match_fn.py`; the LLM (or human) writes the C, the scripts do the
rest.

Run from Git Bash in the repo root with the cross toolchain and the disc set:

```bash
export MIPS_PREFIX=mipsel-none-elf-          # <prefix>objdump must be on PATH
export AC1_BIN="/c/.../Armored Core (USA) (v1.1).bin"
python tools/loop/inventory.py               # ~1 s, writes tools/loop/functions.tsv
python tools/loop/rank.py --unit ovl201      # 20 easiest open functions in overlay 201
```

The carved overlays (`disc_map/overlays/*.bin`, from
`tools/extract/extract_overlay.py`) and the FDAT entries
(`disc_map/extracted/FDAT_T/entries/NNN.bin`) are gitignored; `inventory.py`
looks for them in this checkout, then in the main checkout when run from a git
worktree, then in `$AC1_DISC_MAP`.

### Calibration: program bodies of 1 KB and up (2026-10-20)

After the zero-agent sweep, 186 open program bodies of 1024 B or more remain in
queue-S1..S6 (409 KB, 257 functions with twins; median 1.6 KB, 14 calls,
32 branches; 41 use the GTE, 34 have jump tables, 60 are over 2 KB). One
session matched twelve of them by hand with the match-function loop, one at a
time. Wall time runs from the first disassembly to the oracle's MATCH and
includes reading the asm, writing headers and every oracle run. A sweep is one
parallel batch of source variants through `match_fn.py`, using a small
replace-and-compile helper (now shared as `tools/match/variants.py`).

| # | unit vaddr | bytes | members | wall | oracle runs | sweeps (variants) | what blocked it |
|---|---|---:|---:|---:|---:|---|---|
| 1 | ovl202 0x8006AAC8 | 1524 | 6 | 4 min | 1 | – | nothing: GTE subdivider (POLY_GT3); time went into `render.h`/`gte.h` |
| 2 | ovl202 0x8006B384 | 2220 | 6 | 1 min | 1 | – | quad sibling of #1 |
| 3 | ovl202 0x80069A5C | 1192 | 3 | 1 min | 2 | – | statement order (`setlen` after the colour copy) |
| 4 | ovl202 0x8006A13C | 1712 | 3 | <1 min | 1 | – | sibling of #2/#3 |
| 5 | ovl202 0x800705D8 | 1064 | 3 | 4.5 min | 3 | – | loop-invariant scratchpad base needs a local pointer, not a constant macro; one-expression sum (register); `if (p) {...; return &x;} return 0;` (block layout) |
| 6 | ovl202 0x80070168 | 1136 | 3 | 5 min | 4 | 2 (12) | by-value 2D points must be a 4-aligned 16:16 bitfield struct (DVECTOR goes to the stack); one shared `dd` for three range checks (global-alloc register) |
| 7 | ovl202 0x80060B54 | 1360 | 3 | 2 min | 3 | 1 (8) | globals 0x801A5F24/28 are one struct (in-struct aliasing forces the reloads); statement and operand order |
| 8 | ovl202 0x80070A58 | 1240 | 3 | 4 min | 4 | 2 (14) | spill-slot layout follows declaration order; sign/abs as `s = d > 0 ? (a = d, 1) : (a = -d, -1)`; clip code shared through a `goto`, not cross-jumping |
| 9 | ovl201 0x8008B094 | 1236 | 1 | 9 min | 6 | 7 (~63) | cursor global is a struct member (base register when used twice, no load hoisting); `if (vy <= 0) {...} else vy -= 24;` so the stores cross-jump; local types change the frame; loop-counter init; compare operand order |
| 10 | ovl202 0x8005156C | 1092 | 1 | 2.7 min | 4 | 2 (96) | near-copy cluster n019, from scratch: scalar global is a struct member (base register); `-a + 512`; one variable for the count and the loop index; statement order |
| 10a | ovl203 0x80052238 | 1092 | 1 | 50 s | 2 | – | **port of #10**: view globals become a pointer chain through `D_80039D18`; one assignment order |
| 10b | ovl204 0x80051878 | 1108 | 1 | 20 s | 1 | – | **port of #10a**: new addresses, frame counter read through `D_80039D18` |

Totals: 15.7 KB of bodies, 34 functions credited with twins (12 + 22), about
45 minutes, 32 oracle runs and about 200 sweep variants. No function needed
more than 6 single runs.

- **Porting near-copies.** The open near-copy clusters (masked-word similarity
  0.8 or more; `open_near_twin_clusters_512B.tsv` from openclusters.py) pay off.
  For cluster n019 (#10 → #10a/#10b, three overlays), the first member took
  2.7 min (2.4 min/KB) from scratch. Each port took 20-50 s (0.3-0.75 min/KB)
  and 1-2 oracle runs, 4-8 times faster: copy the C, rename the globals and
  callees, adjust what the asm diff shows. Same-overlay siblings behave the
  same way (clusters n000/n001 are #1→#3 and #2→#4).

- **Hours per KB.** Measured: 0.05 h/KB overall (3 min/KB). A body whose
  family is already done runs at about 0.5 min/KB (#2-#4); a first-of-family
  body at 3-7 min/KB (#1, #5-#9). #9 is the only body shaped like the median
  of the 186 (ovl201, about 11 calls and 27 branches) and took 7 min/KB.
- **Sample bias.** The picks favoured the highest `n_members` without jump
  tables, so 8 of 9 are regular ovl202 render/collision library code. None
  used a jump table, none was over 2.2 KB, and only one came from ovl201,
  which holds 79 of the 186. For the whole set, plan on 5-10 min/KB
  (about 35-70 session-hours for 409 KB, or about 4-12 h of wall time across 6-8
  parallel sessions). The 60 bodies over 2 KB (up to 25 KB) and the 34
  jump-table bodies will likely cost more per KB; m2c still cannot build
  their switches (see the sweep notes).
- **Blockers, in order of cost.** (1) Types and global layout: which
  globals are struct members, by-value structs held in registers (#6, #7, #9).
  (2) Register allocation set by variable identity: shared variables,
  declaration order and spill slots, narrow local types (#6, #8, #9).
  (3) Block layout and cross-jumping: if/else polarity, `goto`-shared code,
  return placement (#5, #8, #9). (4) Statement and operand order under the
  scheduler (#3, #7, #9). (5) Loop shape: invariant base, counter init (#5, #9).
  The GTE was never a blocker: the PsyQ `inline_c.h` shapes in
  `include/ac1/gte.h` matched on the first try.
- **What made it fast.** Typed headers written before the first oracle run
  (`include/ac1/render.h`, `collide.h`, `gte.h`, DVECTOR/RECT), siblings done
  right after their family's first member, and parallel variant sweeps
  instead of one edit per oracle run.
- `drive.py try` counts only its own runs, so the ledger's `attempts` for #3
  and #5 says 1. The `matched ...` commit messages give the real counts (2, 3).

## Units

| unit | image | load address | `match_fn.py` flag |
|---|---|---|---|
| `exe` | PS-X EXE carved from `$AC1_BIN` sectors 39–121 | 0x80011000 (code 0x80011E64–0x800315B4) | none (exe addresses are unambiguous) |
| `ovl201` | `disc_map/overlays/ovl201_front_end.bin` | 0x8004ADA0 | `--ovl 201` |
| `ovl202` | `ovl202_mission.bin` | 0x8004ADA0 | `--ovl 202` |
| `ovl203` | `ovl203_mode1.bin` | 0x8004ADA0 | `--ovl 203` |
| `ovl204` | `ovl204_mode2.bin` | 0x8004ADA0 | `--ovl 204` |
| `fdatNNN` | `disc_map/extracted/FDAT_T/entries/NNN.bin` (`--missions`) | 0x801C4B40 (even 0–98, 120, 140), 0x801C8460 (160–168), 0x801B80C8 (180–188) | `--bin <file> --base <addr>` |

The four program overlays share one load address, so a `(unit, vaddr)` pair,
never a bare address, identifies a function everywhere in these tools.
`ledger.py` accepts a bare `201` for `ovl201`.

## inventory.py

Disassembles each unit once with
`<MIPS_PREFIX>objdump -D -b binary -m mips:3000 -EL --adjust-vma=<base>` and
splits the `.text` range into functions:

1. **`.text` range.**  Known for the exe and ovl201 (splat layout); taken from
   `overlays/CODE_UNITS.tsv` when that file exists; otherwise detected: the
   first stack-frame prologue (`addiu sp,sp,-N` followed by `sw ra`), moved
   back to the earliest jal target / stored code pointer in the decodable run
   before it, through the last `jr ra` that ends a run of decodable words.
   Checked against CODE_UNITS.tsv for all 63 units: 62 exact, one start off by
   8 bytes (fdat074).  `--text UNIT=0xSTART:0xEND` overrides.
2. **Spans.**  Every branch, `j`, and switch-table target (`lui/addu/lw … jr`
   with the table read from rodata, entry count from the preceding `sltiu`) is
   a span from its source to its target.  A boundary crossed by any span is
   inside a function.
3. **Candidates.**  The instruction after `jr ra` + delay slot (`ret`), jal
   targets (`jal`), `addiu sp,sp,-N` prologues (`pro`), code pointers stored in
   rodata/data (`ptr`), and for the exe the `F` rows of `ac1_markup.tsv`
   (`mk`).  A candidate with zero crossing spans starts a function.  GCC 2.7.2
   hoists up to a few loads above the prologue, so a prologue within 8 words
   of another candidate confirms that candidate instead of splitting.
   `jr tN` right after `li tN,0xB0` (BIOS thunks such as AddDrv) counts as a
   return.
4. **Fall-through.**  A start whose only evidence is a prologue (`pro`) and
   whose preceding block does not end in `jr`/`j` is the middle of a function
   (GCC scheduled up to 57 words above `addiu sp` in ovl201); it is dropped.
   After this no overlay function ends without a return; the exe keeps 11
   such blocks, all hand-written PsyQ library objects.
5. **Padding.**  Blocks that are only `nop` are merged into the previous
   function.

### Output: `functions.tsv`

| column | meaning |
|---|---|
| `unit` | see table above |
| `vaddr` | first instruction, `0x%08X` |
| `size` | bytes, **including the delay slot after the final `jr ra`** (what `match_fn.py --expect-size` wants) |
| `name` | `ac1_markup.tsv` name for the exe, else `func_<ADDR>` (overlay-range markup names are ambiguous between overlays) |
| `is_psyq` | 1 inside the exe's PsyQ block: crt0 0x80011E64–0x80011F28 and the libraries 0x80016EA4–0x800315B4 (the tail 0x80030368–0x800315B4 is still library: AddDrv-wrapper objects, AddDrv, DelDrv) |
| `instructions` | `size / 4` |
| `branches` | conditional branches + `j` |
| `calls` | `jal` count |
| `jump_tables` | `jr <reg>` dispatches through a switch table |
| `gte_ops` | COP2 instructions (`c2`, `mtc2`, `mfc2`, `ctc2`, `cfc2`) + `lwc2`/`swc2` |
| `stack_size` | frame bytes from the prologue; 0 = leaf without a frame |
| `jt_addrs` | RAM address(es) of the jump tables, for `match_fn.py --rodata` |
| `evidence` | which candidates established the start (`text`, `ret`, `jal`, `pro`, `ptr`, `mk`) |

Current run (2026-10-18, with `overlays/CODE_UNITS.tsv` merged): 3565 functions = exe 694 (563 PsyQ) + ovl201 1091 +
ovl202 619 + ovl203 579 + ovl204 582 (plus 2769 mission-overlay functions with `--missions`).  The four functions already matched in
`re/match/manifest.tsv` come out with exactly the manifest sizes (40, 76, 104,
252 bytes) and the switch table of `func_800505F0` at 0x8004AEEC.

### Mission overlays (`--missions`)

```bash
export AC1_DISC_MAP="C:/Users/bross/OneDrive/Desktop/AC1_RE/disc_map"   # only from a worktree without disc_map
python tools/loop/inventory.py --missions      # ~4 s; adds the fdatNNN rows
```

`--missions` appends one unit per even `FDAT_T` entry after the exe and ovl
rows, which it leaves byte-for-byte unchanged (checked: the exe/ovl rows of
the two runs are identical). Run of 2026-10-18: **58 units, 2769 functions**,
6334 rows in total. The load address per entry (and the alias column) comes
from `overlays/CODE_UNITS.tsv` when that file is in the checkout, else from
`MISSION_TABLE` in `inventory.py`; the two agree on all 58 bases
(`refresh_mission_entries()` warns if they ever differ).  With CODE_UNITS.tsv
merged the only boundary change is fdat074, whose text starts 8 bytes earlier
(0x801C4D48), so its first function is 1008 bytes instead of 1000.

- **fdat168** is byte-identical to fdat166 (`alias_of` in CODE_UNITS.tsv, also
  `cmp`-verified) and is **skipped** with a printed note, so its functions are
  not counted twice; `--units fdat168` still inventories it. Match it as fdat166.
- **fdat188** has no CODE_UNITS.tsv row: a 4096-byte stub whose eight header
  pointers all target one `jr ra; nop` at 0x801B81E8. It is **listed** with that
  single 8-byte function (`STUB_TEXT`), below `rank.py`'s default floor.
- Text ranges are detected when CODE_UNITS.tsv is absent; they agree with it on
  57/58 units. fdat074's CODE_UNITS `text_va` (0x801C4D48) is 8 bytes before the
  first prologue and those two words are data, so detection (0x801C4D50) is
  what `functions.tsv` carries; with CODE_UNITS.tsv present the file's range wins
  and that one function grows by 8 bytes.
- Names are all `func_<ADDR>` (no markup rows above 0x801B8000); every extern
  needs `--defsym`. The oracle takes `--bin disc_map/extracted/FDAT_T/entries/NNN.bin
  --base <addr>` (no `--ovl`); switch tables (`jt_addrs`) sit in the rodata block
  between the header and `.text`. Header, load groups and the mission → entry →
  name table: `docs/MISSION_OVERLAY_CODE.md`.

### Markup cross-check

For the exe the heuristic starts are compared with the markup rows *before*
the rows are merged in:

```
markup rows found by heuristic : 486/857 = 56.7%
heuristic starts in markup     : 486/739 = 65.8%
game code outside the PsyQ block: markup has 1 row(s), heuristic found 134 function(s), 1 in common
```

Read it with the markup's shape in mind: 856 of its 857 exe rows sit in the
PsyQ block, where ~355 rows are Ghidra's object-internal labels (hand-written
assembly, `j` tail jumps, shared epilogues) that are *not* function starts by
control flow, and the game code in the exe has a single markup row (`main`).
The agreement is therefore a statement about library code, which `rank.py`
excludes anyway.  `--no-markup-starts` keeps the pure heuristic for the exe.

## rank.py

```
rank.py [-n 20] [--unit ovl201] [--tsv] [--max-score S] [--min-instructions 3] [--calibrate]
rank.py --missions [...]                 # mission overlays: one function per twin group, by payoff
```

Filters `functions.tsv` to `is_psyq == 0`, not in the ledger as `match` or
`giveup`, at least `--min-instructions` long (default 3: the 8-byte
`jr ra; nop` stubs are real empty functions but not worth a turn; pass 2 to
include them), then sorts by

```
score = 1.00*log1p(instructions) + 1.00*log1p(branches) + 1.00*log1p(gte_ops)
      + 0.80*log1p(stack_size) + 0.25*jump_tables - 0.60*log1p(calls)
```

(`WEIGHTS` dict at the top of the file, clamped at 0).  Calls are subtracted
because a `jal` plus argument setup is the cheapest code to reproduce; a
switch table is a small flat cost (the case layout; `--rodata` is pre-filled by
`drive.py`).  `--tsv` prints the same rows machine-readably, `jt_addrs` included,
for a driver.  `--calibrate` lists every ledger outcome with its score, size
and attempts so the weights can be refit once there are enough rows (e.g.
regress attempts, or the give-up rate, on the log1p features).

### Weights (reviewed 2026-10-18 against 78 outcomes)

`refit.py` fits log(attempts) on the same features (ridge, pure Python,
give-ups = 20 attempts, give-ups with 0 attempts skipped as tool failures)
and prints the current and refit Spearman plus a proposed `WEIGHTS`.

**Round 1** (55 outcomes, scores up to 8.3) was too easy to tell weightings
apart: 43 first-try matches.  The only change then was branches 0.60 -> 1.00
and stack 0.15 -> 0.30 (Spearman 0.63 either way).

**Calibration round** (`queue-C1..C3.tsv`, built by `pick_calibration.py`): 24
functions, scores 5-11, exe and all four overlays, 4 with GTE ops and 3 with
switch tables, no feature-identical twins.  Three matchers, cap 10 tries,
every compile through `drive.py try`.

| queue | matched | give-ups | attempts |
|---|---|---|---|
| C1 | 8 | 0 | 19 |
| C2 | 7 | 1 | 36 |
| C3 | 6 | 2 | 44 |

Give-ups: ovl201 0x80064C18 (GTE, one scheduling difference at +0x190, 668 vs
664 bytes), ovl201 0x8006E148 (temp registers swapped in two row loops) and
exe 0x80012A78 (s8/t1 allocation of two stack arguments).  The matchers'
notes agree that the cost is GCC 2.7.2 behaviour, not size: register
allocation ties, loop shape (for vs do-while, down-counting walking
pointers), merging of identical blocks and block layout, struct-vs-scalar
globals for load order, and frame size.  Two counted tries were wasted on
unchanged sources (exe 0x80016678, exe 0x80012A78); the ledger keeps the
counts as recorded.

Spearman(score, log attempts):

| weights | all 78 | calibration 24 | C1 / C2 / C3 | top-100 open, largest |
|---|---|---|---|---|
| round-1 weights (gte 0.8, stack 0.3, jt 1.5, calls 0.3) | 0.740 | 0.567 | 0.47 / 0.52 / 0.89 | 28 B |
| **now** (gte 1.0, stack 0.8, jt 0.25, calls 0.6) | **0.784** | **0.799** | 0.91 / 0.63 / 0.93 | 28 B |
| literal ridge (gte 1.27, stack 0.77, jt 0, calls 1.83) | 0.767 | 0.767 | | 2236 B |
| size only | 0.721 | 0.512 | | 28 B |

How the weights were set: a grid over gte {0.8..1.5} x jt {0..1.5} x stack
{0.3..0.8} x calls {0.3..0.9}.  Two directions are robust: **jump tables are
cheap** (all four took 1-3 tries) and **frames matter more** (stack 0.8 tops
the grid at every other setting).  GTE and calls are flat across their range
(all within 0.002 of the best), so they get middle values.  The new weights
beat the old on 487 of 500 bootstrap resamples.  Caveat: they were chosen on
the same data they are scored on; the per-queue column is not out-of-sample.
The literal ridge fit is still rejected for its `calls` weight, which ranks a
2236-byte call-heavy function among the 100 easiest.

What the score cannot see: in the calibration round a 10.5 switch function
took 3 tries and a 7.2 leaf took 6.  Within a band, attempts depend on
compiler quirks no static feature captures, so expect the ranking to order
bands well and individual functions loosely.

Top of `rank.py --unit ovl201` today: 3-instruction leaf setters
(`li v0,1; jr ra; sb v0,1(a0)`), then frameless leaves of 4–9 instructions
without branches; framed functions and functions with branches follow.

### Mission mode (`--missions`)

```
rank.py --missions [-n 20] [--unit fdat020] [--tsv] [--max-score S] [--min-instructions 3] [--twins F]
```

For the mission overlays the cost of a function is the same as anywhere else,
but its value is not: `twins.py replay` turns one matched C file into a
manifest row for every member of its group (see twins.py below).  This mode
reads `twins.tsv` instead of walking `functions.tsv` row by row:

- one line per group: the representative (lowest entry, vaddr), or with
  `--unit` the group's member in that unit (groups without one are skipped);
- a group is closed as soon as any member has a ledger row, `match` or
  `giveup`: the body, and so the C, is the same in every member, and a
  matched member's twins are added by `drive.py --replay-twins`;
- `--min-instructions` and `--max-score` apply to the shown member, so the
  8-byte `jr ra; nop` stub group (g0003, 60 members) is hidden by default;
- sorted by **payoff = n_members / (1 + score)**, highest first: manifest rows
  gained per unit of difficulty (`1 +` so a score-0 function still costs a
  turn).  `score` is the ordinary difficulty score above.

Because `score` is logarithmic in size and branches, group size dominates:
the 34 library groups of 31 members (31 still open) fill 28 of the top 45,
and the lowest of them (payoff 2.30) still ranks far above every function
that occurs only once (163 singletons, payoff <= 0.36).  Run of 2026-10-19
(calibrated WEIGHTS, stack 0.8), 388 open groups:

```
   # group   mem payoff unit    vaddr       size instr  br jal  jt stack score
   1 g0001    66  11.13 fdat004 0x801C6734    60    15   0   1   0    24  4.93
   2 g0041    31  10.07 fdat000 0x801C575C    28     7   0   0   0     0  2.08
   3 g0035    31   9.70 fdat000 0x801C56E4    32     8   0   0   0     0  2.20
   ...
   6 g0034    31   9.12 fdat000 0x801C5634    40    10   0   0   0     0  2.40
   7 g0002    62   8.62 fdat000 0x801C72C4    92    23   2   2   0    24  6.19
   8 g0004    39   6.57 fdat002 0x801C675C    60    15   0   1   0    24  4.93
```

The only group ahead of the frameless 31-member leaves is larger still:
g0001 (66 members, a small stub that occurs twice per entry).  `--tsv`
adds `group`, `n_members` and `payoff` in front of the usual columns.  After a
match, `drive.py try <unit> <vaddr>` then `drive.py --replay-twins` closes the
group.

## ledger.py

```
ledger.py record-match  <unit> <vaddr> <attempts>
ledger.py record-giveup <unit> <vaddr> <attempts> <reason...>
ledger.py status                      # matched / given-up counts and mean attempts per unit
ledger.py is-matched <unit> <vaddr>   # exit 0 if matched, 1 otherwise (no output)
ledger.py state <unit> <vaddr>        # match | giveup | open
ledger.py import-manifest [re/match/manifest.tsv]   # seed from the oracle's manifest
```

`ledger.tsv` is append-only (`when unit vaddr status attempts reason`); the
newest row per `(unit, vaddr)` wins, so a later match supersedes a give-up.
`import-manifest` records every row of `re/match/manifest.tsv` as a match,
taking the attempt count from the `matched func_<ADDR> <n>` commit subjects.
`$AC1_LEDGER` points the tools at another ledger file (tests).

## units.py

```
units.py flags <unit>    # exe -> (nothing); ovl201 -> --ovl 201; fdat060 -> --bin <abs path> --base 0x801C4B40
units.py image <unit>    # absolute path of the image (the exe is carved once into tools/loop/cache/)
units.py base  <unit>    # 0x%08X load address of byte 0 of that image
```

`units.resolve(unit)` returns `Unit(name, image_path, base, text_lo, text_hi,
oracle_flags, manifest_unit)`; `manifest_unit` is what goes in the manifest's
second column (`exe`, `201`, `fdat060`).  Imports only `load_unit`,
`find_in_disc_map`, `find_objdump`, `MISSION_ENTRIES` and `load_code_units`
from `inventory.py`.  Mission-overlay bases and aliases come from
`overlays/CODE_UNITS.tsv` when it is checked out, else from the inventory
table; alias units (`fdat168` = `fdat166`) are refused so a function is only
ever matched under one name.

## drive.py

```
drive.py next   [--unit U] [--queue FILE] [--shard k/n] [--max-score S] [--no-m2c] [--dry-run] [--re-dir DIR]
drive.py try    <unit> <vaddr> [--defsym NAME=ADDR]... [--rodata VA] [--no-auto-defsym] [--commit] [--keep] [--re-dir DIR]
drive.py giveup <unit> <vaddr> <reason...>
drive.py status [--queue FILE]
drive.py verify [--manifest FILE] [-j N] [-q]
```

A matcher's turn is `next` -> edit the C -> `try` until exit 0 -> (`--commit`) ->
`next`; `giveup` when the attempt budget is spent.

**`next`** prints a work package and exits 0, or exits 3 when nothing is open:
the `functions.tsv` row with its score, the retail disassembly (`objdump -m
mips:3000`), the GAS that `tools/ghidra/objdump2m2c.py` makes from the
`mips:isa32` listing, the m2c draft (`-t mips-gcc-c --valid-syntax
--deterministic-vars --stop-on-error`, retried with `--no-andor`; a failure is
printed and skipped), the exact oracle command line (`--expect-size` from the
row, `--rodata <first jt_addrs>` marked *verify* when the function has a jump
table) and the C path.  If `re/match/<unit>/func_<ADDR>.c` does not exist it
writes a skeleton (`#include "ac1/ac1.h"`, a header comment with unit /
vaddr / size / score / date, the m2c draft in a comment, an empty function);
that is the only file `next` writes.  With `--queue tools/loop/queue-A.tsv`
the rows come from that queue in order, skipping rows whose state in the
current ledger is not `open` or whose C file already carries a `NOT MATCHED`
header; `--queue` refuses to run unless `$AC1_LEDGER` points at the matcher's
own ledger (the shared `ledger.tsv` is merged by the lead).  Without
`--queue` the candidates are `rank.candidates()` over `functions.tsv` minus
both ledgers, filtered by `--unit`, `--max-score` and `--shard k/n`
(`int(vaddr,16)//4 % n == k`).

**`try`** builds the oracle command from `units.py` flags, `--expect-size`
from `functions.tsv`, `--rodata` (yours, else the row's first `jt_addrs` when
`jump_tables > 0`, with a note), your `--defsym`s and, when the C file
contains `#include "ac1/ac1.h"`, `--syms include/ac1/symbols.tsv --syms
ac1_markup.tsv` (any `--syms` switches off the oracle's implicit markup, so
both tables go in), runs
`match_fn.py --json -` and splits the JSON from the final line.  Unresolved
symbols named `func_<ADDR>` / `D_<ADDR>` are their own address: they are added
as `--defsym` and the oracle is rerun once (`--no-auto-defsym` disables this);
other names are reported as *needs --defsym or a markup name*.  Exit status:

| exit | meaning |
|---|---|
| 0 | clean MATCH: oracle exit 0, `masked_words == 0`, size equals `functions.tsv` |
| 1 | MISMATCH, or a MATCH with masked words (shape only) |
| 2 | ERROR (build failure, bad address, missing toolchain) |

Every `try` appends one line (`iso-time  result  first_diff_offset|-  diff_words`)
to `re/match/<unit>/func_<ADDR>.attempts`; the line count is the attempt
number (the sidecars are gitignored via `re/match/.gitignore`).  The output is
the result line (`result`, `size ours/expected`, `first diff`, `attempts`),
the oracle's own side-by-side diff, and **one** hint picked by rule from the
diff-pattern table of `.claude/skills/match-function/SKILL.md` (in order:
build error; masked MATCH; size differs with/without word diffs; same
instructions with registers swapped; same instructions reordered (exactly, or
up to register renaming); only load/store offsets differ; only a `lui`/low
half differs with a resolved symbol; stack frame / epilogue; `jal` target;
inverted branch; `nop` count; `lh`/`lhu`-style width/signedness; `andi`
narrowing; `sll`/`sra` sign extension; `mult`/`div` vs shifts; switch table
vs compares; drift after the first diff; else the first differing pair).
`--keep` passes `--keep <tmp>/m_<unit>_<ADDR>` and prints the directory.

On a clean MATCH `try` appends the manifest row
`vaddr<TAB><manifest unit><TAB><unit>/func_<ADDR>.c<TAB><size><TAB><extra>`
(extra keeps `--base`, `--rodata`, the two `--syms` for an `ac1/ac1.h`
source, then `--defsym` minus any NAME=ADDR either table already resolves to
the same address, never `--bin`; deduped on
(vaddr, unit); the file's existing line-ending style is kept), records
`ledger.record("match", unit, vaddr, attempts)` in `$AC1_LEDGER` (or
`ledger.tsv`) unless that ledger already says `match`, and prints

```
git add re/match/<unit>/func_<ADDR>.c re/match/manifest.tsv <ledger> && git commit -m "matched func_<ADDR> <attempts>"
```

`--commit` runs it (the ledger is only added when it lives inside the repo).
`--re-dir DIR` points all of `next`/`try`/`giveup`/`status` at another
`re/match`-shaped directory; a clean MATCH there is reported but not recorded.

**`giveup`** records `giveup` with `attempts = sidecar lines` and the reason,
and prepends `/* NOT MATCHED: <reason>, <n> attempts */` to the C file unless
it already has one.  **`status`** lists unit, vaddr, name, score, ledger state
(`giveup*` = header in the C but no ledger row), attempts and the last sidecar
result for a queue (`--queue`) or for every function the ledger or a sidecar
knows, with totals.  **`verify`** is `check_all.sh` in Python: every manifest
row through the oracle with `units.py` flags, `ok`/`FAIL` per row,
`N/M matched`, exit 0 only when all match.

Environment: `AC1_LEDGER`, `M2C_DIR` (default `~/dev/m2c`, then
`~/Applications/m2c`; its `.venv` python is used when present), plus what
`inventory.py` and `match_fn.py` read (`AC1_BIN`, `AC1_DISC_MAP`,
`MIPS_PREFIX`, `AC1_CC1`, `AC1_MASPSX`, ...).

## twins.py (clones: mission overlays, and exe + program overlays)

```
twins.py group  [--scope missions|programs|all] [--functions F] [--out F]
twins.py replay [--scope missions|programs|all] [--manifest re/match/manifest.tsv] [--twins F]
                [--twins-programs F] [--report F] [--dry-run] [--no-ledger] [-j N]
drive.py --replay-twins [replay options]          # forwards to twins.py replay
```

Both commands default to `--scope all`.  The mission scope is described first;
the program scope (`twins_programs.tsv`) follows under *Program scope* below.

The 58 mission overlays link one object library at per-entry addresses, so
most mission functions occur in many entries with only absolute addresses
changed.  `twins.py` makes one matched C file count for every such clone.

**`group`** reads every `fdatNNN` row of `functions.tsv` from its image and
masks exactly the words that carry a relocation: `jal`/`j` targets, a `lui`
of a RAM high half (0x8000..0x8020), and the low half (`addiu`/`ori`, or a
load/store offset) of every instruction whose base register still holds that
`lui`, also through an `addu` with an index (`lui at; addu at,at,v0;
lw v0,%lo(at)`, i.e. switch tables and indexed globals).  Everything else is
kept.  Equal masked bodies form a group; the representative is the member with
the lowest (entry, vaddr).  `twins.tsv` has one line per member:

| column | meaning |
|---|---|
| `group` | `gNNNN`, biggest groups first |
| `rep_unit`, `rep_vaddr` | representative |
| `unit`, `vaddr`, `size` | this member |
| `n_members` | group size |
| `same_slots` / `diff_slots` | relocation targets equal to / different from the representative's |
| `deltas` | the differing targets, `REP>MEMBER(+delta)`, unique, in code order |

Run of 2026-10-18: **2769 functions -> 393 groups**, 163 singletons, 2604
functions with a twin in another entry; 34 groups of 31 members (the common
library), the largest groups (60-66 members) are tiny stubs that occur several
times per entry.  Size distribution, members: groups:

```
1:163  2:62  3:41  4:6  5:11  6:7  7:5  8:5  9:1  10:7  11:13  12:3  14:4  15:2
16:8  17:4  18:1  20:1  21:2  23:1  25:1  26:1  29:3  31:34  32:1  33:1  36:1
39:1  60:1  62:1  66:1
```

The earlier count of 362 (docs/MISSION_OVERLAY_CODE.md) masked *every*
`addiu`/load/store immediate, which also merges functions that need different
C: struct offsets (the three fdat060 matches, bit index from `+0x2D` vs
`+0x06`, are one body under that mask), frame sizes (`addiu sp,sp,-24` vs
`-168`) and `li` constants (100 vs 500, 3000 vs 5000).  21 coarse groups split
here (into 52); every differing word was checked: 27 `li` constants, 8 frame
fields on `sp`, 19 offsets/constants on a register that holds no RAM `lui`;
none is a relocation.

**`replay`** takes every manifest row whose unit is `fdatNNN` and whose source
lives in that unit's own directory (an original match), and compiles the same
C file against each other member of its group with `match_fn.py --bin <member
image> --base <member base> --expect-size <member size>`.  Each
`--defsym NAME=A` (and `--rodata`) of the matched row is moved by the delta of
the member's relocation slot that targets `A` (or the nearest slot at `A+off`,
`off < 0x1000`); a symbol no slot references is kept when it lies outside the
matched unit's image (exe/ovl globals) and is an error inside it.  Per twin:

- **MATCH** (exit 0, final line `MATCH`): a manifest row `vaddr unit
  <shared source> size --base ... --defsym ...` (deduped, sorted as
  `merge_manifest.py` does), and a ledger row `match`, attempts `0`, reason
  `twin of <unit> <vaddr> (<source>)`.  Attempts 0 means unknown, which
  `refit.py` skips, so clones do not flatten the calibration sample; `rank.py`
  and `drive.py next` stop offering them.  `--no-ledger` skips the ledger.
- anything else is listed as `NOT MATCHED` on stdout and in the report with
  the oracle's final line; nothing is silently dropped.

`re/match/twin_replay.tsv` (committed) lists every attempted twin: group,
source, matched-as, unit, vaddr, size, result, oracle final line, the derived
flags.  Re-running is idempotent (rows already present are re-verified, not
re-added).  Exit 0 only when every twin matched.  Run it after every new
mission-overlay match.

Results so far:

| matched C file | twins | MATCH |
|---|---|---|
| `fdat060/func_801C6E08.c` | 30 | 30 |
| `fdat060/func_801C6E44.c` | 30 | 30 |
| `fdat060/func_801C6E60.c` | 30 | 30 |
| `fdat016/func_801C6AE0.c` | 17 | 17 |
| 31 files in `fdat000/` (the 31-member library groups) | 930 | 930 |
| 22 files in `fdat000/ fdat002/ fdat004/ fdat008/ fdat016/` (15..29-member groups) | 398 | 398 |
| 5 files in `fdat000/ fdat002/ fdat004/` (the 32..66-member groups g0001 g0002 g0004 g0006 g0007) | 227 | 227 |
| `fdat002/func_801C6504.c` (g0005, 36 members) | 35 | 35 |

The fdat060 three are relocation-free; `fdat016/func_801C6AE0.c` (48 bytes,
group g0051: one `jal` and one function pointer, matched with
`--defsym func_801CDC7C=0x801CDC7C --defsym func_801CDBD4=0x801CDBD4`) tests
the `--defsym` derivation: 17/17 twins MATCH with per-entry addresses (e.g.
fdat020 `func_801CDC7C=0x801CAA3C`), and the oracle reports MISMATCH when
fdat020 is given fdat016's addresses instead.

The mission library (2026-10-19): all 31 open 31-member groups from
`rank.py --missions` are matched in `re/match/fdat000/` and replayed onto their 30
twins each (1037 replayed rows in all, every one MATCH).  `801C75BC`'s event
dispatch is a compare tree that one `switch` cannot give with this cc1 (`stmt.c`:
count 6 >= `CASE_VALUES_THRESHOLD` 5 and span 9 <= 10 x count emit a jump table);
the source is an if/goto chain laid out like GCC's case tree.

The 15..29-member groups (2026-10-20): 22 of 23 are matched and replayed (398 more
twins).  Most took one or two oracle runs.  The 23rd, `fdat000 801C981C` (4912 B,
the AI movement state machine installed as `fn_move`, 12 jump tables), is matched on
its own branch (`claude/fdat000-801C981C`).

The five largest open groups (g0001 66, g0002 62, g0004 39, g0006 33, g0007 32
members) are small installers and wrappers (60..92 B); all matched in one or two runs
(227 more twins).

### Program scope (exe + ovl201..204)

`group --scope programs` reads the exe's game code (`is_psyq` 0) and the four
program overlays with the same mask and writes `tools/loop/twins_programs.tsv`
(groups `pNNNN`, same columns).  The overlays share the load address 0x8004ADA0,
so members are keyed by (unit, vaddr) as everywhere else; ovl202/203/204 (mission,
mode 1, mode 2) carry most of the same code at shifted addresses.  Run of
2026-10-19: **3002 functions -> 1732 bodies**, 1140 singletons, 1565 functions with
a twin in another unit; 369 groups of 3, 354 of them one function in each of
202/203/204; the largest (41 members) is the 8-byte `jr ra; nop` stub.

Replay differs from the mission scope in two places:

- a twin is compiled with its own unit's oracle flags (none for `exe`, `--ovl 20N`),
  no `--bin`/`--base`; its manifest row has the derived `--rodata`/`--defsym` only;
- names that the row resolves through a symbol table (its `--syms` files, else
  match_fn.py's implicit `ac1_markup.tsv`) are not in the extra column, so they
  cannot be moved like `--defsym`s.  Every table name at or up to 0xFFF below a
  relocation target is tied to the nearest such slot; when that slot's target moved
  in the member, the name is passed as an explicit `--defsym NAME=<moved address>`
  (an explicit `--defsym` wins over the tables).  A wrong guess can only make the
  oracle say MISMATCH.

A group can hold several originals (clones matched independently before twins
existed); each other member is replayed once, from the first original in manifest
order, and a member that is itself an original (its own `func_<ADDR>.c` in its own
unit) is not replayed over at all: its manifest row names its own file, which
check_all.sh compares with the report's source.  `--scope programs` rewrites only the program rows of `twin_replay.tsv` and
keeps the mission rows as they are; the report's `unit` column is the manifest's
(`202`, `exe`, `fdat016`), so check_all.sh's twin check covers both scopes.

Results: the 72 exe/ovl originals of 2026-10-19 replay onto 89 twins, **89 MATCH**
(77 new manifest rows: 49 in 201, 24 in 203/204, 1 in 202, 3 in the exe).
After the sweep pilot (below), the 118 program originals replay onto 229 twins (all
MATCH); `--scope all` replays 182 originals onto 1954 twins, 1954 MATCH.

The committed `twins.tsv` (mission scope) predates the current `functions.tsv` (the
code reproduces it byte for byte from that input; today's input gives 394 groups
instead of 393), so it was not regenerated here: doing so renumbers the groups that
`twin_replay.tsv` cites.

## progress.py

```
progress.py [--manifest F] [--functions F] [--stubs-matched] [--compare]
```

Matched / total per component as a markdown table in the shape of the progress
table in Wyrelade/Armored-Core-Decomp's README (exe game code; program overlays with
`FDAT_201`..`FDAT_204` under them; mission overlays; total).  Totals are the
`is_psyq` 0 rows of `functions.tsv`; matched are distinct manifest (unit, vaddr)
rows that are a `functions.tsv` start, originals and replayed twins alike (rows that
are not are listed separately).  `--stubs-matched` counts the open 8-byte
`jr ra; nop` functions as matched, as his README says his counts do.  `--compare`
reads his README through `gh api repos/Wyrelade/Armored-Core-Decomp/readme`
(numbers only) and prints his badge and both columns with the delta.  The splits
differ (his exe game code is 147 functions, ours 131; missions 2768 vs 2769), so read
the percentages too.  2026-10-19 after this round:

```
| Component | ours | Wyrelade | Δ matched | ours % | Wyrelade % |
| Main executable (game code)  |   10/131  |   88/147  |  -78 |  7.63% | 59.86% |
| Program overlays (FDAT.T)    |  337/2871 | 1156/2871 | -819 | 11.74% | 40.26% |
| Mission overlays             | 1789/2769 | 1239/2768 | +550 | 64.61% | 44.76% |
| Total                        | 2136/5771 | 2483/5786 | -347 | 37.01% | 42.91% |
```

## sweep.py (zero-agent sweep)

```
sweep.py --queue FILE [--queue FILE ...] [--max-size 160] [--limit N] [-j N]
         [--variants ctx,ctx-no-andor] [--try-all]
         [--neighbour-min 0.5] [--neighbour-op-min 0.6] [--neighbour-tries 1]
         [--work DIR] [--report F] [--best-dir DIR] [--dry-run] [--no-replay] [--no-ledger]
```

No LLM in the loop: for each open body of the queue(s), in order, smaller than
`--max-size`, not closed in the ledger and without a C file in `re/match`, draft C
with `tools/match/ctx.py`'s `decompile()` (symbolised asm + the shared-type context)
and m2c `--valid-syntax`, then the `--variants` after it in order until one
matches (default `ctx,ctx-regvars,ctx-knr,ctx-spframe,ctx-fix-frame,ctx-fix,neighbour,neighbour-op`:
`--reg-vars saved`, then `ctx` with m2c's guessed `/* extern */` prototypes
rewritten unprototyped, `T f();`, since m2c guesses arity from live registers and
a later shorter call is a cc1 error, then the `m2cfix.py` fix-ups below, then `neighbour` (below; it also runs on bodies over `--max-size`);
`ctx-no-andor`, `ctx-void` and `raw` (no context, no symbol rewrite) are also
there; `--try-all` keeps going after a match, to measure them).  m2c runs once per
distinct argument list per body; the variants share its output.  A draft is
`#include "ac1/ac1.h"` + `#include "m2c_compat.h"` (our definitions of the `M2C_*`
names and `NULL`, in `tools/match/include/`) + m2c's text, with unused leading
parameters m2c leaves out put back as `s32 argK`.  Each draft goes through the oracle
like `drive.py try` (unit flags, `--expect-size`, `--rodata` from `jt_addrs`, both
`--syms` tables, unresolved `func_`/`D_` names as `--defsym`, rerun once).  A clean
MATCH is written to `re/match/<unit>/func_<ADDR>.c` with a header comment, re-run
through the oracle from there, and only on exit 0 gets its manifest row and a ledger
row (`match`, attempts 0, reason `zero-agent sweep (m2c ctx)`).  Misses are only
reported (`<work>/sweep_report.tsv`), never recorded as give-ups.  At the end
`twins.py replay --scope programs` credits the twins.  Jobs run in a thread pool
whose work is m2c/cc1 subprocesses; the main thread does all manifest/ledger writes.
Each oracle run keeps its work dir under `<work>` (the m2c repair pass reads its
`fn.i`) and removes it afterwards.

The report has, per body, the closest compiling draft: `closeness` = the oracle's
`aligned_words` (retail words difflib pairs, in order, with an equal word of ours)
/ retail words, `positional` = words equal at the same offset (near 0 for most big
bodies: one extra instruction early shifts everything), and `best_variant`.  The
run ends with a per-variant table (tried, drafted, compiled, matched, mean
closeness, closest-for; neighbour drafts count too).  `--best-dir DIR` writes each missed body's closest draft
to `DIR/<unit>_<ADDR>.c` with the variant and closeness in a header comment: the
starting draft for an agent.

### m2cfix.py fix-ups

| variant | m2c args | fix-up |
|---|---|---|
| `ctx-spdecl` | `--stack-structs` | declare every m2c name the body uses but m2c never declared (`sp1A` beside an address-taken `sp18`, `saved_reg_s2`, `unksp3E`), typed from m2c's stack template, else `M2C_UNK` |
| `ctx-spframe` | `--stack-structs` | spdecl, then every `spNN` below the register save area becomes a field of one local `union { s8 b[N]; s32 w; } sp_frame` at `NN - lowest offset` (N from the prologue's lowest `sw $s/$ra`), so cc1 lays the frame out exactly as the retail code |
| `ctx-voidcast` | | a used value of a callee the headers declare `void` goes through `((M2C_UNK (*)())f)(...)`; `*(void *)K` -> `*(M2C_UNK *)K` |
| `ctx-fix` | `--stack-structs` | knr + spdecl + voidcast, then up to 6 rounds of `repair()`: cc1's messages are mapped from `fn.i` back to draft lines (cpp -P drops blank/comment lines; the draft is fn.i's tail) and one rule per message is applied to that line only: undeclared -> declare; `unary *` / void value / void expression -> `*(M2C_UNK *)` on derefs whose operand is not a typed pointer; invalid operands / array subscript -> `(s32)` on pointer locals, `&x`, `D_`/`g_` globals, `*pp`; too few/many arguments -> unprototyped callee cast |
| `ctx-fix-frame` | `--stack-structs` | as ctx-fix with spframe instead of spdecl |
| `ctx-fix-regvars` | `--stack-structs --reg-vars saved` | as ctx-fix on the `--reg-vars saved` output |

`tools/match/include/m2c_compat.h` also gained `u64`/`s64` (m2c names soft-float
double values that way).  Tests: `tools/match/tests/test_m2cfix.py` (text only).

**Fix-up sweep (2026-10-20)**: the 1236 open bodies of `queue-S1..S6.tsv` (all
sizes; 3 of the 1239 had matched in an aborted first run, all `ctx-spframe`),
`--try-all` over eight variants, `-j 22`, 1104 s.  "Compiled" = cc1/as produced an
object (MATCH or MISMATCH); "vs ctx" = mean closeness change on the bodies where
both compiled; "closest" = bodies where it gave the best draft (ties go to the
earlier variant).

| variant | drafted | compiled | matched | mean closeness | vs ctx (n) | closest |
|---|---:|---:|---:|---:|---:|---:|
| `ctx` | 1166 | 835 | 0 | 0.603 | - | 667 |
| `ctx-knr` | 1166 | 895 | 0 | 0.595 | +0.000 (835) | 35 |
| `ctx-voidcast` | 1166 | 846 | 0 | 0.602 | +0.000 (835) | 7 |
| `ctx-spdecl` | 1166 | 924 | 0 | 0.583 | +0.000 (832) | 14 |
| `ctx-spframe` | 1166 | 924 | **13** | 0.618 | **+0.022** (832) | 170 |
| `ctx-fix` | 1166 | **1141** | 1 | 0.554 | +0.000 (832) | 68 |
| `ctx-fix-frame` | 1166 | **1142** | **14** | 0.592 | **+0.022** (832) | 70 |
| `ctx-fix-regvars` | 1166 | 1130 | 1 | 0.547 | -0.011 (823) | 116 |

(70 bodies are m2c failures, the unlabelled jump tables PR #48 addresses.)  New
matches: 14 here + 3 in the aborted run = **17 bodies** (16 first by `ctx-spframe`,
1 by `ctx-fix`; one is ovl201 0x800AAF20, 1636 B, the first body of 1 KB or more the
sweep has matched), replayed onto **17 twins**: **34 functions**.  The frame union is
the one fix-up that changes code: m2c's separate stack locals let cc1 pick its own
slot order, the union pins every slot.  The other fix-ups add compile rate (ctx 835
-> ctx-fix 1141 of 1166) without matches: their drafts start agents from compiling
C instead of cc1 errors.  Mean closeness of the best draft per size: 0.625 under
160 B (320/322 compile), 0.645 at 160-511 B (446/464), 0.533 at 512-1023 B
(235/264), 0.524 at 1 KB and up (146/186, against 51 for `ctx` alone).  At 1 KB and
up the closeness spread is wide (0.1: 11, 0.2: 18, 0.3: 23, 0.4: 16, 0.5: 24, 0.6:
19, 0.7: 16, 0.8: 14, 0.9: 5 bodies), which is what `queue-L1.tsv` sorts on.

**Pilot (2026-10-19)**: the first 100 open program bodies under 160 B in credit
order (`make_queues.py --bodies -N 1`), `-j 20`, ~25 s per pass.

| pass | bodies matched | notes |
|---|---|---|
| 1 | 40/100 | 15 misses were cc1 errors, 11 of them m2c's `NULL` |
| 2 (`NULL` in m2c_compat.h) | +6 of the 60 misses | |
| 3 (unused leading params filled) | +0 of the 54 left | those drafts also drop a 5th (stack) argument |
| **total** | **46/100 (46%)** | 20/43 under 64 B, 20/41 at 64-111 B, 6/16 at 112-159 B |

The 46 matches replayed onto **152 twins** (all MATCH), so the pilot credited
**198 functions** in under two minutes of wall time.  What the 54 misses look like:
50 MISMATCH (45 with 3+ differing words: m2c's types/signedness, stack arguments
to unknown callees, unused parameters, expression order) and 4 cc1 errors (a
`void` callee whose value is used, arithmetic on `void *`, `*` on a non-pointer).
Every one is an ordinary `drive.py next` work package with the draft as a start.

**Full sweep (2026-10-20)**: every open exe/ovl201..204 body (`make_queues.py
--bodies -N 1 --prefix all`, 1614 bodies, no size cap), `-j 20`.

| pass | bodies | matched | wall |
|---|---:|---:|---:|
| 1 `ctx,ctx-no-andor` | 1614 | 367 (all `ctx`; `ctx-no-andor` added 0) | 311 s |
| 2 `raw,ctx-regvars,ctx-void --try-all`, misses < 1024 B | 1061 | 6 (`ctx-regvars` 5, `raw` 1, `ctx-void` 0) | 316 s |
| 3 `ctx-knr` | 1241 | 2 | 131 s |
| **total** | 1614 | **375 (23.2%)** + 111 twins = **486 functions** | ~13 min |

Pass 1 by size: 45.5% under 64 B (66/145), 45.3% at 64-111 B, 44.9% at 112-159 B,
25.4% at 160-255 B, 8.2% at 256-511 B, 0.8% at 512-1023 B, 0/186 at 1 KB and up.
Of the 1239 left: ~900 MISMATCH (805 with 6+ differing words, 48 within 2 words),
~270 cc1 errors (114 undeclared `spNN` stack-struct fields, ~40 a `void` callee
from the headers whose value m2c uses, ~60 `*`/arithmetic on `void *`/non-pointers)
and 70 m2c failures, all an unlabelled jump table (`jr` with no `jtbl` read).  `ctx-knr`
turned ~60 of the "too few arguments" cc1 errors into compilable drafts but only 2
matched; `ctx-no-andor` and `ctx-void` never added a match and are no longer default.

**Jump tables (2026-10-20)**: `ctx.py` now names a switch's table load
`jtbl_<ADDR>` (the `lui; addu idx; lw; jr` pattern, an address in `jt_addrs`, or,
in a function with a register `jr`, an address whose retail word points into the
function: that catches ovl202 0x80053190, whose hoisted table the inventory
missed), labels the case targets and appends the table as `.rodata` read from the
retail image.  All 70 former m2c failures now draft (0 left); re-sweeping S1..S6
matched 1 (ovl201 0x8004EEEC, 252 B).  Of the other 69: 19 MISMATCH (closest
ovl201 0x8007AA04 at 10 words, ovl202 0x80081318 at 17) and 50 cc1 errors that
are m2c output problems, not the tables: 13 `saved_reg_sN`, 9 a `default:` m2c
places outside the switch body, 6 cc1 "invalid t(ype ...)", 5 too few arguments, the
rest undeclared names and struct misuse.

### The `neighbour` variant (near-copies)

Exact twins are replayed already (`twins.py`), but ovl202/203/204 also carry many
*near*-copies: the same source with a constant, a field or a statement changed.  The
`neighbour` variant (last in the default list) turns a matched near-copy into the
draft:

1. **find**: every original manifest row (its own `func_<ADDR>.c`) is read from its
   image and masked with `twins.mask_function` (relocated fields zeroed), one per
   distinct masked body (545 today).  For an open body, the 5 rows of size 0.7x..1.43x
   with the highest masked-trigram Jaccard are scored with difflib's ratio over the
   masked words; the best is its neighbour.  The whole index is rebuilt from the
   manifest at every run in about a second, so it always sees the latest matches.
2. **remap**: the neighbour's C file is copied with its function renamed to the open
   body's.  The two masked bodies are aligned with difflib (equal blocks, and
   same-length replace blocks position by position), which pairs their relocation
   slots.  Every `func_`/`D_<ADDR>` name then moves by the delta of the paired slot that
   targets it (`twins.map_addr`, the same rule as the twin replay: the nearest slot at
   ADDR+off, off < 0x1000).  Table names whose slot moved, and the neighbour's other
   `--defsym`s, are passed as `--defsym` at the moved address (`twins.table_moves`).
   Its `--rodata` (when the body has no `jt_addrs`) and other flags carry over.  Two
   names that would land on one address (a misaligned slot) keep their old addresses,
   so the draft still compiles.
3. **oracle**: like the m2c drafts.  When the only differing words are immediates
   (same mnemonic and registers, no branch/jal/lui), `neighbour+imm` replaces the C
   literal of our value with retail's (also the `v-1` form of a `<=`/`>` compare and
   the negated form of a subtraction) and runs the oracle once more.

It runs for bodies of every size (`--max-size` limits only the m2c drafts), and only
when the similarity is at least `--neighbour-min` (default 0.5).  `--neighbour-tries N`
also tries the 2nd..Nth best.  The report has a `neighbour` column (`SIM UNIT VADDR
SOURCE` of the best neighbour) for every body, tried or not, so a matcher can start
from that C file.  `--dry-run` prints the same column, without the oracle, in about
4 s.  `--work DIR` keeps the remapped drafts (`<unit>_<ADDR>_neighbour.c`).

After every batch of matches, re-run just this variant over the open queues (~1 min
for all 1239 bodies, of which ~20 s is the oracle and the rest is the twin replay):

```
python tools/loop/sweep.py --variants neighbour -j 24 \
    --queue tools/loop/queue-S1.tsv ... --queue tools/loop/queue-S6.tsv
```

**Measured (2026-10-20)**, all 1239 open bodies of `queue-S1..S6` against the 2622-row
manifest.  Best neighbour similarity by size:

| size | bodies | >= 0.9 | >= 0.8 | >= 0.7 | >= 0.5 |
|---|---:|---:|---:|---:|---:|
| < 160 B | 324 | 4 | 10 | 32 | 77 |
| 160-511 B | 465 | 5 | 18 | 28 | 64 |
| 512-1023 B | 264 | 0 | 4 | 7 | 10 |
| 1 KB and up | 186 | 0 | 2 | 4 | 4 |

155 bodies tried, **1 match** (ovl201 0x800B4370 from ovl201 0x80066F68 at 0.900, a
checksum loop whose bound differs: `neighbour+imm`).  The 154 misses compile (no cc1
errors once colliding names are kept) and their best draft is 4 words off for 4
bodies, 6-10 words for 12, 11-20 for 54 and more for 84.  The remap itself checks
out: the differing words in the close misses are real code differences (a clamp
`>= 0x41` vs `< -0x40`, an extra `lbu`/`sll`/`sw` chain), not names.  `--neighbour-tries 3`
added nothing, and no body below 0.9 matched outright.

So today the variant is mostly a starting point.  It pays off as the hard bodies get
matched: the matched corpus has few near-copies of the open bodies, but the open
bodies are near-copies of *each other*.  170 of the 450 open bodies of 512 B or more
sit in clusters of open bodies with pairwise similarity >= 0.8 (e.g. ovl202
0x80069A5C / 0x8006AAC8; ovl202 0x8005804C, ovl203 0x80057A3C / 0x80064268, ovl202
0x80064DC8).  Match one member of a cluster by hand, then re-run the line above: the
others get its C file remapped as their draft (a match when only constants differ,
otherwise a port: the 9 open bodies at 0.9+ today were 1 to 46 words off).

### The opcode-only backup: `neighbour-op` and reference C (2026-10-24)

The masked-word similarity above keeps register numbers and wants sizes within
0.7x..1.43x, so it misses relatives that do the same thing with other registers,
constants or a few more statements.  `Neighbours.ranked(..., mode="opcode")` is the
backup, in the spirit of coddog: the same trigram pre-filter and difflib ratio, but over
`opkey()` sequences (primary opcode, SPECIAL funct, REGIMM rt, coprocessor command;
registers and immediates dropped) and sizes 0.5x..2x.  It is used in three places:

- **`neighbour-op` sweep variant** (in the default list, after `neighbour`): only for a
  body whose best masked neighbour is under `--neighbour-min`, it drafts from the opcode
  neighbours at or above `--neighbour-op-min` (default 0.6), remapped the same way.
- **`drive.py next`** lists up to `--refs` (default 3) similar matched functions as
  reference C, masked neighbours at or above 0.5 first and opcode ones filling the rest
  (`Neighbours.references`), and prints the best one's C up to `--refs-lines` (80).
- **`golem.py tweak --refs`** adds the best reference function's code to the prompt when
  it fits `--max-prompt` (off by default, so bench runs stay comparable).

**Measured (2026-10-24)**, the 202 open non-PsyQ functions against the 1987-body corpus:

| best masked | best opcode-only | functions |
|---|---|---:|
| >= 0.5 | (already has a masked neighbour) | 50 |
| < 0.5 | >= 0.6 | 15 |
| < 0.5 | 0.4 .. 0.6 | 37 |
| < 0.5 | < 0.4 | 100 |

So the backup gives a reference to 15 more functions at 0.6+ (52 at 0.4+).  As drafts
they do not match: `--variants neighbour-op --neighbour-tries 2` on those 15 compiled
all of them, matched none, mean aligned closeness 0.31 (best 0.73, ovl201 0x800B3748).
Its value is the reference C an agent or golem reads, not the draft; the draft is
there because it costs one compile per body and the corpus grows with every match.

## permute.py (permuter tier)

```
permute.py pick (--report sweep_report.tsv ... | --cfile [UNIT:]FILE.c ...) [--max-words N] [--max-ratio R]
                [--max-size N] [--limit N] [--drafts DIR ...]
permute.py run  (same picking options) [-j 4] [--headroom-gb 4] [--no-memgate] [--budget 300]
                [--levers index,bytes,...]
                [--lever-evals 150] [--beam 4] [--no-permuter] [--work DIR] [--no-ledger] [--no-replay]
permute.py setup <unit> <vaddr> <draft.c> <dir>     # one decomp-permuter dir, for a manual run
permute.py cc    <dir> <in.c> -o <out.o>            # its compile.sh backend
```

No LLM in the loop: the near misses the sweep (or a matcher) left behind are ground
against the oracle overnight.  **Inputs**: `sweep_report.tsv` files (the best
`MISMATCH` try of every body still open in the ledger and without a `re/match` C file;
the draft is read from the report's own directory, a `--drafts` dir, or regenerated
with `sweep.draft()`) and `--cfile` near-miss / give-up C files (`re/match/<unit>/func_<ADDR>.c`,
a sweep draft `<unit>_<ADDR>_<variant>.c`, or `UNIT:PATH` for anything with `func_<ADDR>`
in its name; a give-up's own file is allowed in).  They are ordered by differing words
relative to size, `words / (size/4)`.  Each function then gets two stages inside
`--budget` seconds, both scored with exactly the oracle's compile (cpp, cc1 2.7.2, rodata
align fix, maspsx 2.56, as, and match_fn's relocation resolution with the `--syms` tables
and `func_`/`D_<ADDR>` names):

1. **Levers** (`levers.py`): deterministic rewrites of the draft along the divergence
   rows of the match-function skill, beam search (`--beam` best texts per depth,
   `--lever-evals` compiles), scored by differing words + 2 x the size difference in
   words; a rewrite whose compiled bytes were already seen is dropped.  The diff-driven
   levers (`imm`, `cmp`) read each beam node's own retail | ours disassembly
   (`levers.diff_facts`), so a second constant shows up once the first is fixed.

   | lever | rewrite | retail symptom it fixes |
   |---|---|---|
   | `index` | `M2C_FIELD((E + 0x801F0000), T *, 0x37D4)` -> `D_801F37D4[i]` / `*(T *)(D_801F37D4 + (E))` with `extern T D_801F37D4[];` | `addu at,at,v0` vs ours `addu at,v0,at` (indexed global, p12) |
   | `bytes` | `p += 8` / `p + 4` / `E + &D_x` on a typed pointer -> byte arithmetic keeping the pointee type, or the element count | `addiu s1,s1,8` vs ours `,16`; `sll 2` vs `sll 4` (m2c offsets are in bytes) |
   | `ptype` | pointee type of a pointer local/extern among u8/s8/u16/s16/u32/s32/void | `li v0,255` vs `-1`, `lh`/`lhu`, scaled `+=` |
   | `etype` | `extern M2C_UNK D_x;` -> u8/s8/... (m2c's `M2C_UNK` is `s32` here) | `&D_x + 8` scaled x4 |
   | `const` | a literal in its other-signedness spelling (`-1` <-> `0xFF`/`0xFFFF`) | `li` immediates |
   | `swap` | two adjacent statements | register colouring by first use (p01/p02) |
   | `commute` | operands of a top-level `+ \| & ^ *` | commutative operand order (p20) |
   | `args` | at one call site, the caller's leading parameter(s) put in front (`f(x)` -> `f(arg0, x)`) or its next one(s) after; the callee's prototype in the file widened or made `T f();`, else an unprototyped cast | retail `li a1,18` vs ours `li a0,18` before a `jal`: m2c guesses arity from the argument registers set before the call, so an argument passed through in the caller's own register is dropped |
   | `imm` | (diff-driven) a C literal equal to our differing immediate, or to it +- 1, becomes retail's: `li`/`addiu`/`slti`/`andi`/`ori` immediates, load/store offsets, a `lui` as the whole constant it builds with its addiu/ori (address `lui`s skipped), a shift amount and its power of two; `+ v` / `- v` flip with the sign | retail `lui v0,0xa0` vs ours `0x140`, `li a0,4` vs `6`, `addiu ...,-16` vs `16`: near-copies (`neighbour` drafts) whose constants differ |
   | `cmp` | (diff-driven, only when a compare or branch differs) a comparison of the function in its other relational spellings (`<` -> `<=`, `>`, `>=`; `==` <-> `!=`) | `slt`/`bnez` vs `slt` with swapped operands / `beqz` |

   **`args` batch (2026-10-20)**: every open body of `closeness.tsv` at closeness >= 0.85 and
   <= 600 B (145 bodies, drafted again from their `best_variant`), one `tools/match/variants.py`
   batch with the other levers and one with `args` alone: `args` matched **8** bodies (+6
   twins), none of which the other levers matched (`func_80076548(arg0, 0x12)` three times,
   `func_8004FB8C(arg0, arg1)` twice, `VectorNormalSS(arg0, arg2)`); the other levers matched
   12 (+6 twins) that the pilot never reached.  With `func_8007A218` (+2) from the first
   variants.py run: 21 bodies, 35 functions credited in all.

2. **decomp-permuter** ([simonlindholm/decomp-permuter](https://github.com/simonlindholm/decomp-permuter),
   `$PERMUTER_DIR`, default `~/dev/decomp-permuter` with a `.venv` holding `toml` and
   `Levenshtein`) on the best lever text for the rest of the budget, `--stop-on-zero
   --stack-diffs`.  Its `-j N` needs `fork()` and is broken on Windows (its own comment
   in `src/main.py`), so each function gets one single-threaded permuter process and
   `run -j` keeps that many functions going.  `permuter_main.py` runs it with the compile
   step in-process: stock `compile.sh` costs a bash start-up (~0.28 s), a python start-up
   and maspsx's own (~0.25 s) on top of cc1 (~0.19 s) per candidate on this PC, 1.8
   candidates/s; in-process it is ~11/s for one process alone.  The permuter's target.o is
   the retail words under the function's symbol, and each candidate .o has the oracle's
   relocations applied and `.rel.text` dropped, so score 0 means the same words.

A score-0 text (the lever text, or the permuter's function spliced back into it) goes
through `match_fn.py` like `drive.py try` (unresolved `func_`/`D_` names as `--defsym`,
rerun once); only a clean MATCH is written to `re/match/<unit>/func_<ADDR>.c` (header
names the levers or the permuter), verified again from there, and gets its manifest row
and a ledger row (`match`, attempts 0, reason `permuter (...)`).  Misses are only
reported (`<work>/permute_report.tsv`: base/lever scores, lever compiles, permuter base
and best score, iterations, seconds).  At the end `twins.py replay --scope programs`
credits the twins.  `tools/match/tests/test_permute.py` checks that the in-process
maspsx is the subprocess, that a candidate .o holds the oracle's own words, the splice,
the report parsing and each lever.  Do not edit `permute.py` while a run is going: every
new permuter process imports it.

**Pilot (2026-10-20)**: the 48 open bodies whose `ctx` draft was within 2 differing words
in the full sweep's pass 1 (36-572 B), `-j 24`, 300 s per function.

| pass | stages | bodies matched | notes |
|---|---|---:|---|
| 1 | decomp-permuter only | 7 / 29 that ran | 19 jobs lost to an edit of `permute.py` mid-run; 20 654 compiles in 379 s = 54 compiles/s |
| 2 | `index,ptype,etype,const,swap,commute` + permuter | 21 / 41 | 19 by levers alone (most in 2 compiles), 2 by the permuter after levers; 105 compiles/s |
| 3 | + `bytes` | 7 / 20 | 6 by levers alone, 1 by the permuter; 128 compiles/s |
| **total** | | **35 / 48 (73%)** | 25 by levers alone, 10 by the permuter; plus twins (below) |

**Larger near misses**: the open bodies of all three full-sweep passes at `--max-ratio 0.06`
(29 sweep bodies, 12 of them 300-760 B) plus the three saved give-ups (`exe 0x80012A78`
996 B, `ovl201 0x80064C18` 664 B, `ovl201 0x8006E148` 840 B), 600 s and 250 lever compiles
each.  Pass 1 (without `bytes`): **5 / 29** matched, `ovl202 0x80057360` (376 B, 3 words),
`ovl204 0x80056434` (596 B, 7), `ovl201 0x800A9740` (760 B, 3) by one `etype` rewrite each,
`ovl202 0x80053020` (368 B, 5) by the permuter after an `etype` rewrite, `ovl201 0x8008C1F8`
(336 B, 5) by the permuter alone; 125 compiles/s.  Pass 2 (with `bytes`, the 22 left
including the give-ups): 0; 168 compiles/s.  The give-ups stay give-ups (the levers took
`0x80012A78` from 15 to 9 differing words; global-alloc ties are not something the permuter
found in 600 s at ~0.6 candidates/s for a 1 KB function).

All runs together: **40 bodies + 28 twins = 68 functions** credited, every one through
`match_fn.py` exit 0; `drive.py verify -j 24` all MATCH afterwards.

Throughput at `-j 24`: 105-128 oracle compiles/s across the box (24 permuter processes at
2.5-4.5 candidates/s each; the lever stage runs ~5 compiles/s per function under the same
load).  What is left (the 13 pilot misses): register-only diffs the permuter does not
reach in 300 s (`a0`/`a1` for a call m2c gave one argument too few, e.g. `VectorNormalSS(v)`
for a two-argument call), an argument m2c dropped (`li a0,24` in a delay slot), and drafts
whose word count hid a size difference (`ovl201 0x800B6D4C`: ours 8 bytes for 56; the
sweep's diff words only count the common length).  Those are agent work packages.

## memgate.py (memory gate)

```
memgate.py status                # free commit, headroom, the quote model
memgate.py quote SIZE [BUDGET]   # the quote for one body
```

Two permuter runs at `-j 20` and `-j 12` on 1.5-3 KB bodies (2026-10-23) ran Windows
out of commit (WinError 1450, then `MemoryError`) and took the PC down. The worker count
alone does not protect a machine someone is also using, because one job's memory grows
with the body's size and with how long it runs. `permute.py run` therefore goes through
a memory gate (off with `--no-memgate`):

- **Quote.** Each job's expected peak commit is the worst peak in the calibration log
  x 1.25 (at least 256 MB). With fewer than 4 calibration rows, a deliberately
  pessimistic default by size applies: 1 GB + 1 GB per KB of body.
- **Admission.** A job starts only when
  `free commit - (unused part of every running job's quote) - its quote >= headroom`
  (`--headroom-gb`, default 4, or `$AC1_MEM_HEADROOM_GB`); otherwise it waits. `-j` (now
  default 4) still caps the number of jobs on top.
- **Watchdog.** Every 2 s it reads free commit and each job's process tree (private
  bytes of the permuter and its children). Below the headroom it stops the newest job
  (`taskkill /T`); `run` requeues that body once at the back of the queue, and a second
  stop is reported as `memgate stop`. Each finished job's peak goes to the calibration
  log and to the report's `peak_mb` column.

It watches free **commit**, not free RAM: Windows refuses allocations once the commit
charge reaches the commit limit (RAM + page file), even with physical memory free. On a
PC with a small page file, that limit is only a little above the RAM.

The calibration log (`$AC1_MEMQUOTE`, default `~/dev/ac1_work/memquote.tsv`: tool, unit,
vaddr, size, budget_s, seconds, peak_mb) is per-machine data and stays out of the repo.
The quote improves as runs add rows to it.

**Calibration (2026-10-23).** Ten permuter jobs, one at a time, on open near misses
of 368 B - 3.4 KB with 90-1200 s budgets:

| Peak commit per job | Jobs | Bodies |
|---|---:|---|
| 31-64 MB | 7 | 368 B - 2.9 KB, 600-1200 s |
| 138 MB | 1 | 696 B |
| 552-633 MB | 2 | 620 B (90 s), 1.9 KB (1200 s) |

Peak memory follows neither body size nor run time; it spikes on some bodies. So
the quote is the worst case seen: 633 MB x 1.25 = 791 MB per job. At the default
`-j 4` that is about 3.2 GB in the worst case.

The same runs show what parallel jobs buy. One job alone ran ~27 permuter
iterations/s. At `-j 20` each job ran ~5.5/s, about 110/s in all: 20 jobs for
4x the throughput, at the cost of the whole machine.

## doctor.py (one front door)

```
doctor.py check UNIT VADDR [CFILE]      # what kind of miss, why, and the plan
doctor.py fix   UNIT VADDR [CFILE] [--skip permuter] [--save-best DIR] [--dry-run]
doctor.py scan  --list FILE | --cfile UNIT:C ...  [--out census.tsv]
```

There are many tools now (sweep, levers, permuter, regtie, structdiff, golem, twin
replay), and which one fits depends on the kind of miss. The doctor compiles the C once,
exactly like the oracle, and reads the kind of miss off the diff:

| class | what the diff shows | plan (cheapest first) |
|---|---|---|
| `build` | cc1/cpp/as fails (the error is shown) | m2c fix-ups, an agent |
| `match` | byte-identical | commit (`drive.py try`) |
| `twin` | a member of the body's twin group is matched | `twins.py replay` |
| `constants` | only immediates differ | levers (`imm`/`cmp`), regtie, permuter |
| `registers` | only register names differ | regtie, permuter |
| `schedule` | the same instructions in another order | permuter, regtie |
| `mixed` | register names plus a few other words | regtie, levers, permuter |
| `size-small` | <= 16 bytes off, same branch skeleton (a delay slot, a spill) | regtie, permuter, structdiff |
| `size` | another length, different branches | structdiff, regtie, an agent |
| `structure` | same length, different branch skeleton | structdiff, regtie, an agent |

`fix` runs the plan; a tool that improves the text hands the improvement to the next
one, and only an oracle-confirmed clean MATCH (written through `drive.py try`) counts.
Every tool tried is appended to the doctor log (`$AC1_DOCTOR_LOG`, default
`~/dev/ac1_work/doctor_log.tsv`: when, unit, vaddr, class, words, tool, result, note):
outcome data for `router.py`, and later for a fine-tuned model. CFILE may carry oracle
options as `PATH@rodata=VA@noalign` (a jump table's `.rodata` start,
`--no-rodata-align-fix`), the same spelling `regtie.py run --cfile` takes.

Census of the 95 open near misses with an agent draft (2026-10-23): structure 29,
size-small 24, mixed 15, size 12, registers 7, schedule 7, build 1. So about 53 are
tie-like (regtie's ground) and about 41 structural (structdiff's).

## regtie.py (register-tie tier)

```
regtie.py diagnose UNIT VADDR CFILE
regtie.py run --cfile UNIT:CFILE[@rodata=VA][@noalign] ... [--evals 300] [--beam 6] [--dry-run]
```

Most near misses the permuter leaves are register ties: two values with nearly equal
global-alloc priority got each other's registers. `diagnose` compiles like the oracle
plus `cc1 -dl -dg` (dumps only; no code change) and prints which variable sits in the
register retail gave to which, with both priorities. It reads:

- retail vs ours: aligned instructions that differ only in register names;
- `fn.i.lreg`: each pseudo's weighted use count and live length;
- `fn.i.greg`: global-alloc's allocation order; the same insn (by uid) in both dumps
  maps pseudos to hard registers;
- the COFF debug info in `fn.s` (`-gcoff` is a pinned flag): each register variable's register.

Priority is `floor_log2(refs) * refs / live_length` (global.c; refs are weighted by
loop depth), and ties go to the lower pseudo number.

`run` is a beam search per body over edits restricted to the tied variables:
- move a statement that names one 1-3 places (changes the live range);
- retype its declaration;
- **split** a reused variable from a later assignment on (one allocno per variable in
  this GCC, so reuse stretches the live range);
- **reuse** the tied partner's name;
- levers.py's rewrites that touch them.

The score is (differing words, how far the tied priorities are from flipping), so an
edit that moves the tie is kept before the bytes change. About 14 compiles/s on one core.

First match (2026-10-23): ovl203 0x8007AB4C. It was 3 words off after the agent and 20
minutes / 19,583 decomp-permuter iterations. diagnose showed `k` (priority 2.46) losing
a2 to `k2` (3.67), and `run` matched it in 156 compiles: reuse `k2` as the loop-3
counter and split the later `(r->rate * 3) >> 4` value off. The ovl204 twin replayed.

## structdiff.py (structural diff)

```
structdiff.py UNIT VADDR [CFILE] [--fix [--evals 200] [--dry-run]]
```

It compiles like the oracle, keeps the object and reads its line table (cc1 `.loc` ->
`.debug_line`, `readelf --debug-dump=decodedline`), so every one of our instructions has
a C line. It then compares the two functions with register names abstracted and
reports:

- the first differing region side by side, with its C lines;
- named findings: loop tested at the top vs the bottom, jump table vs compare chain,
  return paths, blocks cross-jumping merged or kept, calls merged, branch sense
  (arms swapped), jumps landing elsewhere (delay-slot stealing / a label inside a
  shared block), and the stack frame size.

Each finding names the idiom that usually fixes it. `--fix` searches structural moves
at the flagged lines (invert an if/else, while <-> guarded do-while, merge a shared
last statement out of both arms, copy the next statement into both) and commits a
score-0 text through `drive.py try`.

Findings over the 41 structural near misses (2026-10-23, bodies with each):

| finding | bodies |
|---|---:|
| jump lands elsewhere | 20 |
| loops | 7 |
| tail | 6 |
| branch | 6 |
| loop | 5 |
| frame | 5 |
| call | 3 |

14 had no named finding. The moves matched none outright in a first pass, but took
ovl203 0x8007CB90 from 418 to 118 words and ovl202 0x800873F8 from 217 to 187;
`doctor.py fix` hands such improvements on to regtie.

## nearport.py (near-copies)

```
nearport.py scan [--min 0.97]                   # families of open functions with the same shape
nearport.py port OUTDIR [UNIT:VADDR ...]        # port matched C onto open copies, one compile at a time
```

twins.py groups functions whose bodies are identical once relocation slots are masked.
Copies that differ in a few immediates fall into separate groups, so a match is never
replayed onto them. The usual difference is one struct field offset: the renderer reads
its globals block at +0xB20 in ovl202 and at +0x670 in ovl203/204. Two agents can then
work the same body twice, and an agent may port a twin by hand when a tool could do it.

- **scan** links open functions whose opcode/funct sequences are at least `--min` alike
  (or that share a twin group) into families, and names the closest matched function for
  each. A family is one unit of work: give it to one agent, then port or replay the rest.
- **port** takes each open function whose shape is identical to a matched one. It copies
  that C, renames the function, and moves its symbols by the relocation slots (the
  twins.py wide-window rule). It then tries single-literal edits that turn the matched
  copy's differing immediates into the open copy's, such as a pad size before a field.
  Register differences and sources that need `--rodata` are reported, not tried. A clean
  MATCH stays in OUTDIR, with a promote item `unit:0xVADDR@NAME=ADDR...`.

First run (2026-10-24, 113 open): 6 of 12 shape-identical copies ported with no agent
time, worth 6 functions. Of the 101 open functions, 23 families had a split twin group or a
matched near-copy.

## golem.py (local LLM tier)

```
golem.py fix   UNIT VADDR FILE.c [--model M] [--attempts 3] [--out F] [--dump DIR]
golem.py tweak UNIT VADDR FILE.c [--model M] [--rounds 3] [-n 6] [--out F] [--dump DIR]
golem.py run   (--report sweep_report.tsv|permute_report.tsv ... | --cfile [UNIT:]F ...)
               [--job auto|fix|tweak] [--max-words 12] [--max-size 800] [--no-commit]
golem.py bench BENCH.tsv --models A,B --out DIR
```

A small model served by LM Studio (`--url`/`$GOLEM_URL`, default `http://localhost:1234/v1`,
any OpenAI-compatible `/chat/completions`; `--model`/`$GOLEM_MODEL`) in a closed loop with
the oracle.  Each answer is scored with the oracle's own compile (permute.py's in-process
pipeline, variants.py's batch: differing words + 2 per word of size difference), and a
score-0 text goes through `match_fn.py`; only a clean MATCH is written to `re/match`
(header names the job and model), re-verified there, and gets its manifest and ledger
rows (`golem (<model>, <job>)`).

- **fix**: a draft cc1 rejects.  The prompt has the cc1 errors under the draft lines they
  are about (`m2cfix.map_errors`), the header prototypes of the names the code uses, the
  target assembly when it fits, and the code after the `#include`s; the answer is the
  whole code in one ```` ```c ```` block.  Up to `--attempts`, each from the last answer's errors.
- **tweak**: a draft within `--max-words`.  The prompt has the retail | ours listing
  (positional when the sizes agree, difflib-aligned otherwise; the whole listing up to 64
  instructions, else the differing regions), one hint line (`drive.pick_hint`'s rules, and
  the two constants when a `lui`/`li` immediate is what differs), the C, and the edits
  already tried; the answer is up to `-n` `## label` / `-old` / `+new` blocks (the
  `variants.py --edits` format).  Blocks whose `-` lines are assembly are dropped.  All
  blocks, and pairs of the improving ones, are compiled in one `variants.run_batch`; the
  best improvement is the next round's input.

Prompts stay under `--max-prompt` characters (~4K tokens).  Qwen3 gets `/no_think` and
`<think>` blocks are stripped.  Every call is a row of `--log` (job, model, tokens, seconds,
score before/after; default `~/dev/ac1_work/golem_log.tsv`); `--dump DIR` keeps every
prompt and answer.  `overnight.sh` runs `golem.py run --job tweak` on the deep pass's
`permute_report.tsv` when `GOLEM=1` and the server answers.  Tests:
`tools/match/tests/test_golem.py` (a mocked endpoint: answer parsing, tweak and fix
reaching a MATCH on the worked example, the server being down).

**Bake-off (2026-10-21)**: Intel Arc A750, LM Studio 0.4 with the Vulkan llama.cpp runtime,
Q4 GGUF models; prompts of 1-4K tokens are answered at roughly 25-40 tokens/s (a 2K-token
prompt plus 300 tokens of answer: 9 s for qwen2.5-coder-7b, 17 s for qwen3-8b).  Fixed
budget: tweak 3 rounds of up to 6 edits, fix 3 answers.  63 rows:
A = 20 matched functions whose m2c draft (ctx-spframe / ctx-fix-frame) is 1-12 words off
(the known answer is never shown); B = the 23 open bodies within 11 words that the
overnight permuter left (120 s quick + 300-900 s deep pass, levers included); C = 20 open
`ctx-spframe` drafts cc1 rejects (60-500 B, spread over error kinds).

| | A: matched (1-12 words) | B: open near misses (1-11 words) | C: compile failures | wall |
|---|---|---|---|---|
| permute.py (levers + decomp-permuter) | **11/20** in 120 s each (10 by levers) | 0/23 (that is how they were picked) | n/a (needs a build) | CPU, -j 24 |
| `ctx-fix-frame` rule-based repair | - | - | 14/20 compile (2 within 12 words) | seconds |
| qwen2.5-coder-7b-instruct | 0/20, 1 better | **3/23 matched**, 4 better | 9/20 compile (3 of the rules' 6 failures, 67-88 words off) | 48 min (40-54 s/row) |
| qwen3-8b (`/no_think`) | 0/20, 0 better | 1/23 matched (one of the 3), 3 better | 13/20 compile (1 of the rules' failures; beats the rules on 2: 86 -> 39, 91 -> 41 words) | 45 min (35-55 s/row) |

The 3 matches (ovl201 0x8006AAC4, ovl201 0x8008AFA8, ovl204 0x80060B08; with the twin
replay 4 functions) are all `neighbour` drafts whose remaining difference was a constant
or a sign: `+ 0x10` / `>=` -> `- 0x10` / `<=`, `ui_obj_alloc(6)` -> `(4)`, `0x1400000` ->
`0xA00000` (a `lui` immediate, which `neighbour+imm` does not rewrite).  On register
allocation, statement order and type/lever diffs neither model found anything; qwen2.5
often proposes edits to the assembly (dropped).  Compile fixes are no near misses: the
LLM-fixed drafts are about as far off as the rule-based ones (median 55-80 words; the
same 2 rows within 12), out of reach of tweak and of the permuter.

**Recommendation**: run **tweak** with qwen2.5-coder-7b-instruct after the deep permuter
pass on what it left within 12 words (`GOLEM=1 bash tools/loop/overnight.sh`): GPU only,
~40 s per body, ~15-20 min for a night's leftovers, and the only tier so far that has
closed constant/sign diffs in near-copies.  Do not use **fix** in the pipeline: the free
rule-based repair compiles as many drafts (14 vs 13 and 9) and the results are not close enough to
matter.

**Follow-up (2026-10-22): the `imm` / `cmp` levers.**  All three golem wins were constant/sign
differences, so `levers.py` now has two diff-driven levers (permute.py table above).  On the same
rows, `lever_search` with `imm,cmp` alone matched **4/23 of tier B in ~15 s of CPU** (the golem's
3 plus ovl201 0x80087EB8, two constants `4 -> 2`, `2 -> 1`; ovl201 0x80051A74 went 8 -> 1 words),
and the full default set (old levers + `imm,cmp`) keeps tier A at 9/20 and adds those 4.  They are
in `DEFAULT_LEVERS`, so `permute.py`, `overnight.sh` and `variants.py --levers all` use them; a
`permute.py run --levers imm,cmp --no-permuter` pass over the overnight sweep's 39 closest open
drafts matched 0x80087EB8 (the wider pass over all ~445 compiling drafts was stopped early with
nothing more).  With constants handled for free, the golem's remaining edge on this bench is zero:
keep `GOLEM=1` off until a prompt change (below) shows wins the levers cannot reach.  Ideas for
a better golem: map each differing instruction to its C line (cc1 line info) and ask for
edits by line number; few-shot examples from the ledger's lever trails; a model
fine-tuned on oracle-generated (broken C + diff -> repairing edit) pairs.

### router.py (tier router, 2026-10-22)

```bash
python tools/loop/router.py labels      # router_labels.tsv: tier that closed each ledger row + label_source
python tools/loop/router.py features    # router_features.tsv: functions.tsv + image-derived + draft closeness
python tools/loop/router.py eval        # holdout + 5-fold CV split by twin group; accuracy, regret, windows
python tools/loop/router.py rank --report WORK/sweep/sweep_report.tsv --permute-report WORK/perm2/permute_report.tsv
```

Tiers: 1 sweep / levers, 2 decomp-permuter, 3 golem, 4 Claude short (<= 2 tries),
5 Claude long, 6 open.  Labels come from the ledger `reason`, the `matched func_<ADDR> N`
commit subjects and the `.attempts` sidecars; batch-promoted rows (`matched N ... functions`,
wave 2/3) have an unknown try count and say so.  Twin rows are never trained on and a twin
group never straddles train and test.  Learners: scikit-learn when it imports (`eval` in
seconds), else pure Python (a binned CART tree and a small boosted ensemble, about a
minute); `--backend python|sklearn` forces one and the numbers agree within a point.
Results and the wiring recommendation: docs/ROUTER_SCOPE.md.  Short version:
"a machine tier closes it" vs "Claude" is learnable (0.85 accuracy before any CPU is spent,
0.95 once a draft's closeness is known); short vs long Claude is not (0.6-0.7); the open set
is a residue the machine tiers have already failed on, so routing alone saves no windows
there, and the actionable rows are the 33 agent near misses (`near_misses.tsv`, permuter
input) and the 16 giants (block-region splitting).

## viewer.py (watch a run, leave hints)

```bash
python browser-tools/match-viewer/viewer.py                    # the newest overnight.sh work dir, http://localhost:8767/
python browser-tools/match-viewer/viewer.py WORK --replay      # play a finished work dir back
```

On Windows, `browser-tools\match-viewer\viewer.cmd` is a double-click launcher (point a desktop shortcut
at it, *Run: Minimized* keeps the console out of the way): it starts the viewer on the
newest work dir and opens the page; launched again while one is running, it only opens
the page.

A stdlib-only local server (127.0.0.1) and one page (`viewer.html`) that polls it every
1.5 s. It works in any browser and in the Claude desktop app's browser pane; the theme
follows the system (light / dark) and the ◐ button pins either.

**Top bar.** The overnight stage from `overnight.log` (queue › sweep › permuter quick ›
permuter deep › twins › verify › end), the tool working right now (the sweep variant
being drafted, `levers`, `permuter`, `twins`, `verify`, with how many bodies are in each),
and counters: bodies tried, near misses, matches, compiles/s, elapsed.

**Function list (left).** Every body of the run's queue (`all1.tsv`) in queue order:
status (`open`, `tried` = drafted by the sweep, `near` = picked for the permuter,
`MATCH`), the best differing-word count, a sparkline of the scores so far (sweep variants,
then the lever search's best-so-far), its TU and systems from
`tools/archaeology/tu_map.tsv`, and ★ / skip / give up and ✎ for your hints. Filter by
text or status. A click pins a body; unpinned, the view follows the body the run started
last.

**Main view.** The C sources this run has for the body (each sweep draft, the lever
text, the permuter's best spliced back into it, a score-0 result) as tabs, the newest
permuter text (or the sweep draft with the fewest differing words) selected. Three
columns: the retail asm, that C, and our compiled asm, from the oracle's own side-by-side
(`match_fn.py` with the `drive.py try` auto `--defsym` rerun, once per distinct text, in a
background thread). Differing words are red, the differing operands darker, masked
(relocated) words amber; the two asm columns scroll together. Below: the score history
(sweep words per variant, the lever best-so-far, the permuter's base and new bests).

**Your insight.** Per function: a free-text hint ("draws the radar", "loops over the 8 AC
parts"), a proposed `name` (an identifier, or a prototype `void radar_draw(UiObj *o)`, which
also gives the C type) or `field` (`AcEntity +0x4C hp`), and a triage flag `priority`,
`skip`, `give up` (`clear` removes it). They go to the tracked `re/match/hints.tsv`
(`unit vaddr kind text timestamp`, append-only; the newest triage row wins; × deletes a
row) through `POST /api/hints`, and are read back by:

- `make_queues.py`: bodies flagged `skip` / `giveup` are dropped, `priority` ones go first
  (`--bodies`, also with `--closeness`); a flag on any twin counts for the body;
  `--no-hints` ignores them. The overnight pass builds its queue this way, so the sweep
  and the permuter follow your triage on the next run.
- the match-function skill: an agent runs `python tools/loop/hints.py show <unit>
  0x<VADDR>` (and for the twins) before its first draft.
- `python tools/loop/hints.py names`: every name proposal as the `include/ac1/symbols.tsv`
  row it would become, with what blocks it (no C type yet, name or address already taken),
  plus the field proposals to put into `include/ac1/*.h` by hand. Nothing is applied until
  `hints.py names --apply NAME ...`, which appends those rows, runs `ctx.py gen` and
  `ctx.py check`, and restores both files when check fails. Commit `symbols.tsv` +
  `symbols.h` on their own.

**Human controls** (`human.py`), for someone who knows the game but not MIPS or GCC:

- *Try something.* The diff is sorted into plain-language symptoms (same instructions /
  different registers, instructions in a different order, an extra or missing instruction, a
  different constant or offset, a load of the wrong size or sign, a branch flipped, a
  different function called), each with the moves worth a try: the `levers.py` families in
  plain words (swap neighbouring statements, flip the sides of `+ & | ^ *`, pass a dropped
  argument through, count pointer steps in bytes, change what a pointer points at, give a
  global a byte or short type, write a constant the other way, name a raw address as an
  array) and "let the permuter chew on it for N minutes". One click runs the move on the C
  shown, in the background: every site of that lever is compiled with the oracle's own
  pipeline (as `tools/match/variants.py` does, in process), the best one is kept as *your
  try N* in `<work>/human/<unit>_<ADDR>/` when it scores better or the same with different
  code, and the job line shows the score change in green or red. *Undo my last try* shelves
  the newest try. A score of 0 is only called a MATCH after `match_fn.py` says so, and only
  *Record your try N as a MATCH* writes `re/match`, the manifest row and a ledger row
  (re-verified from there first); commit those like any match and run `twins.py replay`.
- *Point and say.* Click an asm line and the C line it came from lights up (cc1's `.loc`
  lines read back with `objdump -l` from the oracle's `fn.o`, the preprocessed body lined up
  with the C); click two C line numbers and *Swap* tries them the other way round.
- *Pipeline steering* next to the hints: Work on this next (`triage priority`), More
  deep-pass time (`steer deep`: the next overnight deep pass takes it on top of the
  `DEEP_N` closest), Skip for now (`triage skip`), Send to agent / golem (`steer agent` +
  `triage priority`), Give up, Clear. ⏸ in the top bar writes `<work>/PAUSE`: sweep and
  permute finish the bodies they are on and hold before the next (`heartbeat.wait_if_paused`);
  ▶ removes it.
- *What am I looking at?* One paragraph on the current diff, from the same classifier.

Every try, undo, record, steer and pause is logged to `re/match/actions.tsv` (`timestamp
unit vaddr action detail before after result`), so later passes can see which human moves
worked. The try endpoints run compiles, so the server refuses any `--host` but 127.0.0.1.

**Live data.** `sweep.py` and `permute.py` print only their last lines into `overnight.log`
until a stage ends (`| tail`), so both keep `<work>/<stage>/current.json` (`heartbeat.py`):
the bodies in flight with their phase (variant, levers, permuter, oracle), live lever /
permuter scores and iterations, every finished body's status and words, and the compile
count the compiles/s comes from. A heartbeat older than 30 s that never closed shows as
STALE. Everything else (drafts, `levers.log`, `permuter.log`, `output-<score>-<n>/`, the
reports) is read from the work dir directly.

**Replay.** `--replay` puts a play / pause / speed (10×-600×) / scrub bar under the top
bar and plays the work dir back on a virtual clock: every event has a file time (a sweep
draft written, a permuter dir set up, the lever search done, a new permuter best found),
the stage times come from `overnight.log`, and lever evaluations / permuter iterations are
spread over their stage for the compiles/s.

**Recording it as video.** Nothing in the viewer records; use a screen recorder on the
browser window. On Windows, the Game Bar (Win+Alt+R starts and stops recording the active
window; files land in `Videos\Captures`) is enough for one window. OBS Studio
(obsproject.com) records a chosen window or region at a set frame rate: add a *Window
Capture* (or *Browser* source pointed at `http://localhost:8767/`), then Start Recording. A
replay at 60× fits a 30-minute overnight pass in half a minute.

Tests: `python -m pytest tools/match/tests/test_viewer.py tools/match/tests/test_viewer_human.py`
(parsers, the work-dir model at a replay time, the heartbeat and pause, `hints.tsv` round-trip
through the HTTP endpoints, the queue triage, the symptom classifier, the line map, versions /
undo / the action log; one lever try end to end when the toolchain is set up).

## check_all.sh

`re/match/check_all.sh` re-verifies every manifest row with the oracle and
prints `ok`/`FAIL` per row and `N/M matched` (exit 0 only when all match).
It strips a trailing `\r` from every field (the manifest checks out with CRLF
under `core.autocrlf=true`; an empty extra column used to arrive as `"\r"` and
fail with *unrecognized arguments*), passes `--ovl` only when the unit column
is numeric, nothing for `exe`, and for `fdatNNN` rows adds
`--bin "$(python tools/loop/units.py image $unit)"` (plus `--base` from
`units.py` when the extra column has none).  `drive.py verify` is the same
check without bash, and `drive.py verify -j N` (default: CPU count) runs the
oracles in parallel with the output in manifest order (`-q`: FAIL rows and the
total only): 1938 rows in 81 s at `-j 24` against ~12 min serial.  Replayed twin
rows (a row whose source is in another unit's directory: `fdatNNN`, or a program
twin under `exe`/`201`..`204`) are verified like every other row and counted on a
separate line; in addition every `MATCH` row of `re/match/twin_replay.tsv`
must still have its manifest row (same vaddr, unit and source), else that is a
`FAIL`.

## make_queues.py / merge_manifest.py

`make_queues.py [--units ovl201,ovl202] [--out-dir DIR]` rebuilds the matcher
queues: `rank.candidates()` over the units, sorted by (score rounded to 3
decimals, size), cut into bands `< 2.5`, `2.5-4.5`, `4.5-6.5`, from which
2x14, 2x6 and 2x4 rows are taken at `round(i*(n-1)/(k-1))` across each band's
pool and dealt alternately to A and B.  The default is a dry run that prints
both queues; `--out-dir` writes them, and the committed `tools/loop/queue-*.tsv`
are only overwritten with `--force`.  It reproduces the committed queues byte
for byte.

`make_queues.py --bodies [-N 6] [--twins F] [--max-size 1000] [--no-tier2] [--limit K]
[--prefix queue-S] [--out-dir DIR]` is the sprint mode: the unit of work is a masked
body (a group of `twins_programs.tsv` by default), because one matched C file is
replayed onto all its members.  Open bodies (no member in the ledger) come from
`rank.mission_candidates()`, one row per body (its representative), ranked by
**credit per effort = n_members / (1 + score)**; tier 1 is `gte_ops == 0` and
size <= 1000 B, tier 2 (GTE or larger) follows.  The ranked list is dealt snake-wise
(1..N, N..1, ...) into N disjoint queues, so no body is in two queues and every
queue gets the same credit profile.  Rows carry `group n_members payoff tier` before
the usual columns; `drive.py next --queue` and `sweep.py` read them.  The committed
`queue-S1..S6.tsv` were written after the fix-up sweep and the neighbour sweeps
(2026-10-20): 1209 open bodies (924 tier 1, 285 tier 2), 201-202 per queue, 2046 functions
credited if all were matched.  Every body in them has had the machine drafts (m2c,
the m2cfix fix-ups, and the neighbour variant where one is >= 0.5) tried and missed.

`make_queues.py --bodies --closeness FILE [--min-size N]` orders the open bodies by
**closeness x n_members** instead (then payoff), dropping those under `--min-size`
bytes; FILE is a sweep report or `tools/loop/closeness.tsv` (the fix-up sweep's
report without the `tries` column), and rows get `closeness best_variant` appended
(empty when no variant compiled; those sort last).  The committed `queue-L1.tsv` is
`--closeness tools/loop/closeness.tsv --min-size 1024 -N 1 --prefix queue-L`: 173
bodies of 1 KB and up still open after the 1 KB calibration, 222 functions, 134 with a
compiling draft (mean closeness 0.517).  The same bodies are also in `queue-S*` (tier 2); take a body from one
queue only.  To get the best drafts back: `sweep.py --queue tools/loop/queue-L1.tsv
--max-size 100000 --best-dir DIR` (default variants; `--limit K` for the first K
bodies, or a one-row queue file and `--variants <best_variant>` for one body).

`merge_manifest.py [MANIFEST] [--check]` reads `re/match/manifest.tsv` even
with git conflict markers or duplicate rows, keeps the union, dedupes on
(vaddr, unit), sorts by (unit, vaddr) with `exe` first, then 201..204, then
`fdatNNN`, and rewrites the file with LF endings and the `#` header kept.

## How the loop fits together

```
inventory.py  ──► functions.tsv ─┐
                                 ├──► rank.py --tsv -n 1 --unit ovl201 ──► (unit, vaddr, size, jt_addrs)
ledger.tsv ◄── ledger.py ◄───────┘             │
      ▲                                        ▼
      │          m2c draft  ──►  write re/match/<unit>/func_<ADDR>.c
      │                                        │
      │          python tools/match/match_fn.py <vaddr> <file.c> --ovl N --expect-size <size> [--rodata <jt_addrs>]
      │                                        │
      │              exit 0 + final "MATCH"  ──┼──►  git commit -m "matched func_<ADDR> <attempts>"
      └──  record-match <unit> <vaddr> <n>  ◄──┘     (also append to re/match/manifest.tsv)
      └──  record-giveup <unit> <vaddr> <n> <why>  after the attempt budget, exit 1 "MISMATCH" or exit 2 "ERROR"
```

Contract with the oracle: only `match_fn.py` exit status 0 with a final line
`MATCH <fn> <n>` is a match; `MISMATCH` (exit 1) and `ERROR` (exit 2) are not.
A `MATCH` printed with `WARNING: n word(s) masked` is a shape match only, so
the driver should pass `--defsym` for the unresolved symbols before recording
it.  `size` from `functions.tsv` is what `--expect-size` wants (delay slot
included), and `jt_addrs` is what `--rodata` wants.

Regenerate `functions.tsv` whenever the split heuristics change (it is
committed so `rank.py` works without the disc); the ledger is keyed by
`(unit, vaddr)`, so a re-split that moves a start simply leaves the old row
unmatched in the inventory.

## Not done / follow-ups

- Weights were set on the data they are scored on.  The next matching round
  is the out-of-sample check: compare `refit.py` Spearman before and after it.
- exe functions that read `.sdata` gp-relative cannot come out of the pinned
  `-G0` flags; the C3 matcher used `register u32 *gp asm("$28")`, which matches
  but is not original source.  A `-G` setting per exe file would fix it.
- `drive.py next` hides runs of zero words in its disassembly (GTE `nop`s
  show only as an address gap); `drive.py try` counts a try even when the
  source is unchanged since the last one.
- Give-ups worth a second look: ovl201 0x80064C18 (one delay-slot fill short).
- Twin replay (`twins.py`, above) covers 63 mission matches (1697 twins); every
  group of 15 or more members is done (801C981C on its own branch), so
  `rank.py --missions` moves on to the groups below 15 members.  A group can have
  several members in one unit (g0001: fdat004 6734, 6A28 and 6D80); `twins.py`
  only treats the row whose address names the C file as the original.
- `ledger.tsv` keeps two rows for functions the inventory has since re-split
  (`ovl201 0x800A86B0` fragment, `ovl202 0x8004EADC` give-up at the old
  20-byte size); the ledger is append-only, `refit.py` skips keys that are not
  in `functions.tsv`.
- `tools/mission/type_catalog.py` / `mission_parse.py` assume one mission base
  and compose lui/lo16 without sign extension (see docs/MISSION_OVERLAY_CODE.md).
