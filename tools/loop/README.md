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

Current run (2026-10-18): 3565 functions = exe 694 (561 PsyQ) + ovl201 1091 +
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
(`refresh_mission_entries()` warns if they ever differ).

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
```

Filters `functions.tsv` to `is_psyq == 0`, not in the ledger as `match` or
`giveup`, at least `--min-instructions` long (default 3: the 8-byte
`jr ra; nop` stubs are real empty functions but not worth a turn; pass 2 to
include them), then sorts by

```
score = 1.00*log1p(instructions) + 0.60*log1p(branches) + 0.80*log1p(gte_ops)
      + 0.15*log1p(stack_size) + 1.50*jump_tables - 0.30*log1p(calls)
```

(`WEIGHTS` dict at the top of the file, clamped at 0).  Calls are subtracted
because a `jal` plus argument setup is the cheapest code to reproduce; a
switch table is a flat cost because it needs `--rodata` and the right case
layout.  `--tsv` prints the same rows machine-readably, `jt_addrs` included,
for a driver.  `--calibrate` lists every ledger outcome with its score, size
and attempts so the weights can be refit once there are enough rows (e.g.
regress attempts, or the give-up rate, on the log1p features).

Top of `rank.py --unit ovl201` today: 3-instruction leaf setters
(`li v0,1; jr ra; sb v0,1(a0)`), then frameless leaves of 4–9 instructions
without branches; framed functions and functions with branches follow.

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
writes a skeleton (`#include "ac1_types.h"`, a header comment with unit /
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
`jump_tables > 0`, with a note) and your `--defsym`s, runs
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
(extra keeps `--base`, `--rodata`, `--defsym`, never `--bin`; deduped on
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

## check_all.sh

`re/match/check_all.sh` re-verifies every manifest row with the oracle and
prints `ok`/`FAIL` per row and `N/M matched` (exit 0 only when all match).
It strips a trailing `\r` from every field (the manifest checks out with CRLF
under `core.autocrlf=true`; an empty extra column used to arrive as `"\r"` and
fail with *unrecognized arguments*), passes `--ovl` only when the unit column
is numeric, nothing for `exe`, and for `fdatNNN` rows adds
`--bin "$(python tools/loop/units.py image $unit)"` (plus `--base` from
`units.py` when the extra column has none).  `drive.py verify` is the same
check without bash.

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

- `drive.py try` auto-adds `--rodata` from `jt_addrs` for jump-table
  functions; that is the table address, which is only the object's `.rodata`
  start when the table comes first.  Verify it against the retail `lui/addiu`
  pair when such a function refuses to match.
- `drive.py` has no attempt budget of its own: `giveup` is a decision the
  matcher takes (the skill says ~15 attempts without progress).
- The exe image for `units.py image exe` is carved into `tools/loop/cache/`
  (gitignored by `*.bin`); delete it to re-carve.
- Weights are hand-set; refit from `rank.py --calibrate` once the ledger has a
  few dozen rows.
- Mission overlays (`--missions`) are inventoried but their names are all
  `func_<ADDR>` and the oracle needs `--bin/--base` for them.
