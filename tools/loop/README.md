# tools/loop — ranking and bookkeeping for the matching-decompilation loop

Three small Python tools that decide *which* function to try next and record
*what happened*.  They do not compile or compare anything: the only judge of a
match is the oracle, `tools/match/match_fn.py` (final line `MATCH ...` or
`MISMATCH ...`, exit status 0 only on MATCH).

```
inventory.py  -> functions.tsv   every function in the exe + overlays, with difficulty features
rank.py       -> stdout          unmatched, non-PsyQ functions, easiest first
ledger.py     -> ledger.tsv      append-only outcomes: match / give-up per function
```

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
| `fdatNNN` | `disc_map/extracted/FDAT_T/entries/NNN.bin` (`--missions`) | 0x801C4B40 (even 0–98, 120, 140), 0x801C8460 (160–168), 0x801B80C8 (180–186) | `--bin <file> --base <addr>` |

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
   hoists up to a few loads above the prologue, so a prologue within 4 words
   of another candidate confirms that candidate instead of splitting.
   `jr tN` right after `li tN,0xB0` (BIOS thunks such as AddDrv) counts as a
   return.
4. **Padding.**  Blocks that are only `nop` are merged into the previous
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

Current run (2026-10-18): 3582 functions = exe 701 (567 PsyQ) + ovl201 1094 +
ovl202 622 + ovl203 581 + ovl204 584.  The four functions already matched in
`re/match/manifest.tsv` come out with exactly the manifest sizes (40, 76, 104,
252 bytes) and the switch table of `func_800505F0` at 0x8004AEEC.

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

- The driver itself (m2c draft → C file → oracle → ledger) lives in the
  `match-function` skill and is not scripted here.
- Weights are hand-set; refit from `rank.py --calibrate` once the ledger has a
  few dozen rows.
- Mission overlays (`--missions`) are inventoried but their names are all
  `func_<ADDR>` and the oracle needs `--bin/--base` for them.
