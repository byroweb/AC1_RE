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
- the retail function size (next function start − VADDR; from the disassembly or
  Ghidra), to pass as `--expect-size`,
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
   file; `#include "ac1_types.h"` gives `u8/u16/u32/s8/s16/s32`). Declare externals
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
   | `lui/addiu` or `lui/lw` immediates differ, word marked `m` | an external symbol: pass `--defsym NAME=0x...`, or the symbol has the wrong address |
   | epilogue / stack frame differs (`addiu sp,sp,-N`, `sw ra`) | number of locals that live across calls, or an `s` register you are not using/using; a `static` or an extra parameter |
   | `jal` target differs | wrong callee name/address; `--defsym` it or check `ac1_markup.tsv` |
   | retail has a `switch` table and yours has `beq` chains (or the reverse) | the retail source used `switch` (dense cases) vs `if` chains; also try reordering cases |
   | everything matches except after a point, then total drift | a missing/extra early `return` or an inverted condition earlier in the function |

   Do not "fix" by inline asm, by `#pragma`, by changing the pinned flags, or by
   editing `match_fn.py`. If a diff makes no sense after a few attempts, compare
   against `/tmp/m_<VADDR>/fn.s` (what cc1 emitted) to separate compiler decisions
   from maspsx/assembler ones.

5. **On MATCH** (exit 0, no mask warning, size as expected): commit the C file with
   the message
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
