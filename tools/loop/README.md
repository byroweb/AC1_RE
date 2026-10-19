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
make_queues.py                   rebuilds queue-A.tsv / queue-B.tsv from rank.py
merge_manifest.py                dedupes + sorts re/match/manifest.tsv after a merge
twins.py      -> twins.tsv, re/match/twin_replay.tsv
                                 mission-overlay clones: group by masked body, replay a matched C file on its twins
```

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
drive.py verify [--manifest FILE]
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

## twins.py (mission-overlay clones)

```
twins.py group  [--functions F] [--out tools/loop/twins.tsv]
twins.py replay [--manifest re/match/manifest.tsv] [--twins F] [--report F] [--dry-run] [--no-ledger] [-j 4]
drive.py --replay-twins [replay options]          # forwards to twins.py replay
```

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

The fdat060 three are relocation-free; `fdat016/func_801C6AE0.c` (48 bytes,
group g0051: one `jal` and one function pointer, matched with
`--defsym func_801CDC7C=0x801CDC7C --defsym func_801CDBD4=0x801CDBD4`) tests
the `--defsym` derivation: 17/17 twins MATCH with per-entry addresses (e.g.
fdat020 `func_801CDC7C=0x801CAA3C`), and the oracle reports MISMATCH when
fdat020 is given fdat016's addresses instead.

## check_all.sh

`re/match/check_all.sh` re-verifies every manifest row with the oracle and
prints `ok`/`FAIL` per row and `N/M matched` (exit 0 only when all match).
It strips a trailing `\r` from every field (the manifest checks out with CRLF
under `core.autocrlf=true`; an empty extra column used to arrive as `"\r"` and
fail with *unrecognized arguments*), passes `--ovl` only when the unit column
is numeric, nothing for `exe`, and for `fdatNNN` rows adds
`--bin "$(python tools/loop/units.py image $unit)"` (plus `--base` from
`units.py` when the extra column has none).  `drive.py verify` is the same
check without bash.  Replayed twin rows (an `fdatNNN` row whose source is in
another unit's directory) are verified like every other row and counted on a
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
- Twin replay (`twins.py`, above) covers four mission matches (107 twins).
  The cheapest mission work is now the 31-member library groups: one match
  there is worth 31 manifest rows; `rank.py --missions` lists them first.
- `ledger.tsv` keeps two rows for functions the inventory has since re-split
  (`ovl201 0x800A86B0` fragment, `ovl202 0x8004EADC` give-up at the old
  20-byte size); the ledger is append-only, `refit.py` skips keys that are not
  in `functions.tsv`.
- `tools/mission/type_catalog.py` / `mission_parse.py` assume one mission base
  and compose lui/lo16 without sign extension (see docs/MISSION_OVERLAY_CODE.md).
