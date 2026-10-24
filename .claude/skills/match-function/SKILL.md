---
name: match-function
description: Byte-match one Armored Core (SLUS-01323) function. Use when asked to "match", "decompile to matching C", or "get a MATCH on" a function at an address in the exe or an overlay (201..204). Drives tools/match/match_fn.py in a write-C / compare / fix loop and commits only on exit 0.
---

# match-function

New to the terms (near miss, twin, family, lever)? See `docs/MATCHING_GUIDE.md`.

Goal: a C function that, compiled with the retail toolchain, produces exactly the
retail bytes at the given address. The only judge is

```bash
python tools/match/match_fn.py <VADDR> <file.c> [--ovl N] [--syms FILE.tsv ...] [--defsym NAME=ADDR ...]
```

**Only exit status 0 with a final line `MATCH <fn> <n>` counts as matched.**
Not "looks equivalent", not "m2c says", not "only register names differ". If the
tool says MISMATCH, the function is not matched, whatever the diff looks like.
A MATCH printed with `WARNING: n word(s) masked` is a match of the code shape
only; resolve the symbols (`--defsym`) before calling it done.

## Inputs you need

- the address `VADDR` and where it lives: exe (`0x80011000..0x8004ADA0`) or an
  overlay (`--ovl 201` front end, 202 mission, 203/204 modes; all load at
  `0x8004ADA0`, so say which),
- the retail function size (next function start minus VADDR, **including the delay
  slot after the final `jr ra`**; from the disassembly or Ghidra), to pass as
  `--expect-size`,
- the toolchain: `tools/match/vendor/toolchain.json` must exist, else run
  `tools/match/setup.sh` once (minutes; builds cc1 on Windows).

## Read the human hints first

The person running the loop knows the game and leaves notes per function in
`re/match/hints.tsv` (through `browser-tools/match-viewer/viewer.py`). Before the first draft, read the
hints for your function **and its twins** (same body, other address or unit):

```bash
python tools/loop/hints.py show <unit> 0x<VADDR>       # e.g. ovl201 0x8007CECC
```

- `hint`: what the function does ("draws the radar", "loops over the 8 AC parts").
  Use it to pick the struct type, the loop shape and the local names.
- `name` / `field`: a proposed name for the function or a struct field. Use it in
  your C as a comment or local name; a proposal becomes a real `symbols.tsv` name
  only through `python tools/loop/hints.py names` and review (see below), never by
  editing `symbols.tsv` on its own say-so.
- `steer agent`: the person handed this body to an agent lane on purpose; their
  tries are in the overnight work dir's `human/<unit>_<ADDR>/` (`versions.tsv` says what
  each move did) and `re/match/actions.tsv`, worth reading before you start.
- `triage`: `priority` (do it first), `skip` or `giveup` (the person does not want
  machine time spent on it: stop and report rather than grind). The queues from
  `make_queues.py` already apply these.

Hints are claims about behaviour, not about bytes: the oracle still decides.


Matched code shares one set of struct headers and one name table (details and
the field-level evidence: `include/ac1/README.md`):

- `#include "ac1/ac1.h"` gives `u8`..`s32`, the PsyQ `SVECTOR`/`VECTOR`/`MATRIX`,
  and the structs: `UiObj` (0xD0, overlay 201 front-end object), `AcEntity`
  (0x170, overlay 202 actor), `Projectile` (0xE0) and `WeaponDef` (0x1C), plus
  `MissionCtl` / `WorldStruct`. Each ends in a size assert. The `extern`
  declarations for every named global and function come from the generated
  `symbols.h`, so do not redeclare them.
- `include/ac1/symbols.tsv` (address, name, C type, unit) holds the shared
  names. Look an address up there (and in `ac1_markup.tsv` for exe/PsyQ names)
  before you invent a `func_<ADDR>` / `D_<ADDR>` placeholder.
- To add a name: add a row to `symbols.tsv`, run `python tools/match/ctx.py gen`
  (regenerates `symbols.h`), then `python tools/match/ctx.py check`. Commit the two
  files separately from the `matched ...` commit.
- A source that includes `ac1/ac1.h` needs both tables on the oracle line:
  `--syms include/ac1/symbols.tsv --syms ac1_markup.tsv`. Any `--syms` switches
  off the implicit `ac1_markup.tsv`, so always pass both. `tools/loop/drive.py try`
  adds them by itself when it sees the include.

## Loop

1. **Carve + decompile.**  Get a typed first draft with the shared context:
   ```bash
   python tools/match/ctx.py m2c 0x<VADDR> --unit 201 --proto "void func_<VADDR>(UiObj *o);"
   ```
   It carves the bytes (length from `tools/loop/functions.tsv`, or pass it), turns
   `jal` targets and `lui`/`%lo` pairs found in `symbols.tsv` into names, and runs
   m2c (`mips-gcc-c`) with `ac1/ac1.h` as context, so fields print as `o->update`
   instead of `*(s32 *)(arg0 + 0x74)`. `--proto` types the function itself; use
   `UiObj *` in 201, `AcEntity *` or `Projectile *` in 202. `--raw` gives the
   untyped draft that `tools/ghidra/overlay2c.sh` prints. Keep the raw disassembly
   next to you; the diff below is in the same objdump syntax.

2. **Write the C** into `re/match/<ovl or exe>/func_<VADDR>.c` (one function per
   file). Start from `#include "ac1/ac1.h"` and the struct types above; the matched
   sources in `re/match/` that include it are the examples (e.g.
   `re/match/ovl201/func_8007CECC.c`). Names from `symbols.tsv` and
   `ac1_markup.tsv` resolve through `--syms`; anything else is `extern` as
   `D_<ADDR>` / `func_<ADDR>` with `--defsym D_<ADDR>=0x<ADDR>` (or add it to
   `symbols.tsv`, see above). Jump-table functions (`jr` through a table) need
   `--rodata <VA of the table>`.

3. **Run the oracle** with `--expect-size` and `--keep /tmp/m_<VADDR>` so cc1's
   `fn.s` and the two `.bin` slices stay around:
   ```bash
   python tools/match/match_fn.py 0x<VADDR> re/match/ovl201/func_<VADDR>.c --ovl 201 --expect-size <SIZE>        --syms include/ac1/symbols.tsv --syms ac1_markup.tsv --keep /tmp/m_<VADDR>
   ```
   Exit 2 (`ERROR:`) is a broken build (syntax, missing symbol, toolchain), not a
   near miss: read the toolchain log it prints, fix, rerun.

4. **Read the diff**, change one thing, rerun. Common patterns:

   | diff looks like | usually means |
   |---|---|
   | same instructions, registers swapped (`v0`↔`v1`, `a1`↔`a2`, `s0`↔`s1`) | local variables declared/first-used in a different order, or a temporary that should be a named local (or vice versa) |
   | same instructions, different order; stores/loads reordered | statement order in the source differs from retail (GCC 2.7.2 keeps source order of side effects; see the perturbed test) |
   | missing or extra `nop` after a load | load-delay slot: a value used right after its load. Restructure so the use is further away, or the use is actually of a different variable; `volatile` on a struct field/global changes this too |
   | `lh` vs `lhu`, `lb` vs `lbu`, `lw` vs `lh` | wrong field type (signed vs unsigned, width); remember `-funsigned-char`: plain `char` is unsigned |
   | `addiu v0,v0,N` vs `addu` with a register, or `sll`+`addu` chains | wrong struct stride / array element size, or index arithmetic written on a different type (`int` vs `short`) |
   | wrong immediate on `lw rX,N(base)` | wrong field offset or wrong struct type. If the retail bytes prove a field in `include/ac1/*.h` wrong, fix the header (pads keep the size assert) and rerun `re/match/check_all.sh`, since other matches use it; do not cast pointer arithmetic |
   | `andi ..,0xff`/`0xffff` present or missing | implicit narrowing: an expression computed in a narrower type, or an assignment to a `u8/u16` field |
   | `sll`/`sra` pair present or missing | sign extension from a `s8/s16` value, or a `char`/`short` parameter being promoted |
   | `mult`/`div` vs shift-and-add sequences | multiply/divide by a constant vs a variable, or signed vs unsigned operands |
   | `lui/addiu` or `lui/lw` immediates differ, word marked `m` | an external symbol: pass `--defsym NAME=0x...` |
   | a single `lui` (or `addiu`/`lw` offset) differs, everything else matches | the `--defsym` address is wrong; recompute it from the retail `lui`+`addiu`/`lw` pair (negative low halves!) |
   | `0 differing word(s)` but `size: ours N, expected M` | your size is wrong (forgot the delay-slot `nop`, or the retail function continues), or your C is a strict prefix of the retail function |
   | epilogue / stack frame differs (`addiu sp,sp,-N`, `sw ra`) | number of locals that live across calls, or an `s` register you are not using/using; a `static` or an extra parameter |
   | `jal` target differs | wrong callee name/address; `--defsym` it or check `ac1_markup.tsv` |
   | retail has a `switch` table and yours has `beq` chains (or the reverse) | the retail source used `switch` (dense cases) vs `if` chains; also try reordering cases |
   | everything matches except after a point, then total drift | a missing/extra early `return` or an inverted condition earlier in the function |
   | **GCC 2.7.2 divergence families** (tags = probe in `tools/match/probes/` that reproduces the remedy through `match_fn.py`; *unverified* = no probe yet) | |
   | register coloring: `v0`↔`v1` (or the two `subu`/`and` operands) swapped, same instructions | swap the two statements that compute the values: allocation follows first use, declaration order alone changes nothing (p01, p02) |
   | `s0`↔`s1` swapped for values kept across calls | s0 goes to the value with more uses per live range; at equal priority the later-defined value gets s0 (two params: x→s1, y→s0). Add/drop a use or swap the defining statements (p03, p04) |
   | retail `lui/addiu rB` then `l* rX,N(rB)` (N≠0); ours a `lui`+`%lo` per field | `p = &G; p->f` alone does **not** do it: GCC folds a known pointer back to direct access (p05). A base survives only where CSE cannot fold it: pointer set before a join and used after it (p07); `G.a += G.b` plus clamp stores to both after joins, plain globals (p06); fields re-read after a join, where the field read *first* becomes the base and the rest go relative (read `cur` first for `670/666/0($b)`, p08); a store CSE cannot see through between the entry reads and the re-reads, e.g. `{ S *p = &H; p->x = 0; }` (p09); an extern-array element RMW in a branch plus a re-read after the join gives `A[-16]` as `-32($b)` (p11) |
   | retail field direct (`lui; lw %lo`) while ours shares a base with the other fields | retail accessed it as a **scalar**: declare `extern T D_<field addr>;` and use that name; scalars are plain symbol addresses and never join a base (func_80079370 `prev`/`vel`/`timer`; p09 A) |
   | the base form is right after the first CSE pass but gone in ours | the CSE rerun (`-frerun-cse-after-loop`, on at -O2) refolds `base+const` when it knows the base. Confirm with a throwaway `--cc1` wrapper adding `-fno-rerun-cse-after-loop` (diagnostic only, never a match), then change the source per the row above (func_80079370 diagnostic run) |
   | retail `move rA,rB` in a branch delay slot, then `and ..,rA` in the target; ours the `and` in the slot | the field was re-read into a named local *before* the `if`; the CSE rerun turns the load into a copy that combine cannot merge across the block boundary (p10) |
   | indexed global: `lui $at; addu; lw %lo($at)` vs `lui/addiu; addu; lw 0` | split form = one access to that array in the block, la form = two or more (p12). `static` vs `extern` changes only the relocation (match_fn masks a static's own `.bss`: no `--defsym` for it) (p13) |
   | every direct access has its own `lui` | normal: `lw $r,sym` is an assembler macro expanded per access; a shared `lui` with several `%lo` offsets is a base-register case above. *Observed in every probe, no dedicated probe* |
   | `addiu sp,sp,-8` / `-16` with no `sp` loads or stores | empty slots: an `s16` local (or reloaded `s16`) live across a branch costs 8 (p14; `int` costs nothing), an unused local array its size (p15), a `for`/`while` loop 8 (p22). Count these before inventing a dummy array |
   | epilogue: retail `lw ra; sw ..; jr ra; addiu sp` (filled), ours `addiu sp; jr ra; nop` | filled only when `ra` is the only saved register and the last statement is one word that can sit after `lw ra` (a store through a pointer). A final global store (`lui $at` macro), a final load, or a saved `s` register gives the unfilled form (p16, p17) |
   | frameless leaf: `jr ra; move v0,..` vs `jr ra; nop` | a single-exit result variable fills the slot; early `return`s leave `nop` (p18) |
   | one shared tail reached by `j` from several arms vs a copy per arm | early `return`s with identical tails are cross-jumped into one; a result variable with `else if` keeps one tail per arm (p19) |
   | `and v0,a1,a0` vs `and v0,a0,a1` | commutative operands keep source order: swap them (p20) |
   | OR/AND chain shaped differently | no reassociation: parenthesise like retail, `a|b|c|d` = left chain, `(a|b)|(c|d)` = balanced (p21) |
   | loop: `blez n` guard (with `i = 0` in its slot) + bottom test + empty 8-byte frame | `for`/`while`. No guard: `do {} while`. Guard without the frame: `if (n > 0) do {} while` (p22, p23) |
   | loop counts down (`addiu -1; bgez`) vs up | GCC reversed it because the index only feeds stores; forward needs an order dependency, e.g. a volatile target (p25) |
   | invariant load before vs after the loop guard | a global read in the body is hoisted to after the guard; a local copy taken before the loop is loaded before it (p24) |
   | `nop` in a loop-branch delay slot where a store could go | a store to a bare global is a two-word `lui $at` macro and never fills a slot; `volatile` makes no difference here (p26) |
   | `nop` before `l* $r,%lo(sym)($base)` right after a load of `$base` | ASPSX keeps the load-delay `nop` for this form; our patched maspsx emits it (`tools/match/patches/`, `tests/test_maspsx_patch.py`). If ours lacks it, rerun `tools/match/setup.sh` |

   When the diff suggests several candidate fixes (statement order, a type, operand
   order), try them in one batch instead of one oracle run each:
   `python tools/match/variants.py 0x<VADDR> <file.c> --ovl N --edits try.txt [--levers swap,commute] [--pairs]`
   (`try.txt`: `## label` then `-old` / `+new` lines). It ranks every variant by
   differing words with the oracle's own compile and re-checks score-0 ones through
   `match_fn.py`; it never edits the file unless `--apply`. The `matched` commit's
   `<attempts>` counts your own `match_fn.py` runs, not the batch's variants.

   Re-check every family row in one go with
   `python tools/match/probes/run_probes.py` (`-v` prints both disassemblies of each
   pair; `pytest tools/match/tests/test_probes.py` runs it too). Before you add a row,
   add a probe pair that reproduces it, and mark the row *unverified* until it passes.
   For a near miss, reproduce the suspect shape in a 5-line probe file through
   `match_fn.py --bin <zeros> --base 0x80100000 --keep DIR` first (without `-q`:
   `ours.bin` is only written with the side-by-side). It costs no attempts on the
   real function and is how the rows above were found.

   Do not "fix" by inline asm, by `#pragma`, by changing the pinned flags, or by
   editing `match_fn.py`. If a diff makes no sense after a few attempts, compare
   against `/tmp/m_<VADDR>/fn.s` (what cc1 emitted) to separate compiler decisions
   from maspsx/assembler ones.

5. **On MATCH** (exit 0, no mask warning, size as expected): add a row to
   `re/match/manifest.tsv` (vaddr, overlay, source, size, the extra `--rodata`/
   `--syms`/`--defsym` args; `drive.py try` writes this row itself and leaves out
   a `--defsym` the tables already resolve), check `re/match/check_all.sh` still says all matched, and commit
   the C file plus the manifest row with the message
   ```
   matched func_<VADDR> <attempts>
   ```
   where `<attempts>` is the number of `match_fn.py` runs it took, e.g.
   `matched func_8004F7E8 3`. Nothing else goes in that commit.

6. Give up cleanly after ~15 attempts without progress: commit nothing, leave the
   best C in place with a `/* NOT MATCHED: <first diff offset>, <what differs> */`
   header, and report the last diff. A partial match is reported as a mismatch.

## Stuck on a near miss? Ask the doctor

After a few tries without progress, let the compiler tell you what kind of miss it is
instead of guessing:

```bash
python tools/loop/doctor.py check <unit> 0x<VADDR> [file.c[@rodata=0x<VA>]]
```

It compiles once like the oracle and prints the class (`registers`, `mixed`,
`size-small`, `structure`, `size`, `constants`, `schedule`, ...), why, and the plan.

- **Register tie** (`registers` / `mixed` / `size-small`): run
  `tools/loop/regtie.py diagnose`. It names the variable in the wrong register, the one
  retail gave that register to, and both allocation priorities. Fix the tie, not the
  symptom:
  - split a variable that is reused for an unrelated later value;
  - reuse one variable for two loops;
  - move a statement to change a live range.

  `regtie.py run --cfile <unit>:file.c` searches those edits automatically.
- **Structural** (`structure` / `size`): run `tools/loop/structdiff.py`. It points at the
  C lines where the shape diverges and names the pattern: loop tested at the top or the
  bottom, jump table vs compares, a tail cross-jumping merged, swapped if/else arms, a
  jump into a shared block. `--fix` tries the standard moves there.

`doctor.py fix` runs the whole plan, cheapest tool first. Only a clean MATCH from
`drive.py try` counts, as always.

## Quick reference

- MATCH / MISMATCH / ERROR line formats and exit codes: `tools/match/README.md`.
- `--json -` prints the facts (`masked_words`, `first_diff_offset`, ...) for a
  loop that wants to parse instead of read.
- Worked example that matches: `tools/match/tests/func_8004F7E8.c`; its deliberate
  mismatch: `func_8004F7E8_perturbed.c`.
