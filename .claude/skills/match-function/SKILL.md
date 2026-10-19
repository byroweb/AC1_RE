---
name: match-function
description: Byte-match one Armored Core (SLUS-01323) function. Use when asked to "match", "decompile to matching C", or "get a MATCH on" a function at an address in the exe or an overlay (201..204). Drives tools/match/match_fn.py in a write-C / compare / fix loop and commits only on exit 0.
---

# match-function

Goal: a C function that, compiled with the retail toolchain, produces exactly the
retail bytes at the given address. The only judge is

```bash
python tools/match/match_fn.py <VADDR> <file.c> [--ovl N] [--defsym NAME=ADDR ...]
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

## Loop

1. **Carve + decompile.**  Disassemble the retail bytes and get a first draft:
   ```bash
   tools/ghidra/overlay2c.sh 0x<VADDR> 0x<SIZE> disc_map/overlays/ovl201_front_end.bin 0x8004ADA0
   ```
   (`overlay2c.sh` runs objdump + m2c with target `mips-gcc-c`; for the exe pass
   the carved exe and base `0x80010800`). Keep the raw disassembly next to you; the
   diff below is in the same objdump syntax.

2. **Write the C** into `re/match/<ovl or exe>/func_<VADDR>.c` (one function per
   file; look at the four matched examples already in `re/match/ovl201/` first; `#include "ac1_types.h"` gives `u8/u16/u32/s8/s16/s32`). Declare externals
   as `extern` with the names from `ac1_markup.tsv` when they exist (then their
   addresses resolve automatically); otherwise name them `D_<ADDR>` / `func_<ADDR>`
   and pass `--defsym D_<ADDR>=0x<ADDR>`. Jump-table functions (`jr` through a
   table) need `--rodata <VA of the table>`.

3. **Run the oracle** with `--expect-size` and `--keep /tmp/m_<VADDR>` so cc1's
   `fn.s` and the two `.bin` slices stay around:
   ```bash
   python tools/match/match_fn.py 0x<VADDR> re/match/ovl201/func_<VADDR>.c --ovl 201 --expect-size <SIZE> --keep /tmp/m_<VADDR>
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
   | wrong immediate on `lw rX,N(base)` | wrong field offset: fix the struct, do not cast pointer arithmetic |
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
   `re/match/manifest.tsv` (vaddr, overlay, source, size, the extra `--defsym`/
   `--rodata` args), check `re/match/check_all.sh` still says all matched, and commit
   the C file plus the manifest row with the message
   ```
   matched func_<VADDR> <attempts>
   ```
   where `<attempts>` is the number of `match_fn.py` runs it took, e.g.
   `matched func_8004F7E8 3`. Nothing else goes in that commit.

6. Give up cleanly after ~15 attempts without progress: commit nothing, leave the
   best C in place with a `/* NOT MATCHED: <first diff offset>, <what differs> */`
   header, and report the last diff. A partial match is reported as a mismatch.

## Quick reference

- MATCH / MISMATCH / ERROR line formats and exit codes: `tools/match/README.md`.
- `--json -` prints the facts (`masked_words`, `first_diff_offset`, ...) for a
  loop that wants to parse instead of read.
- Worked example that matches: `tools/match/tests/func_8004F7E8.c`; its deliberate
  mismatch: `func_8004F7E8_perturbed.c`.
