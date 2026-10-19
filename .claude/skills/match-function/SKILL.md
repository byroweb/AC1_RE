---
name: match-function
description: Byte-match one Armored Core (SLUS-01323) function. Use when asked to "match", "decompile to matching C", or "get a MATCH on" a function at an address in the exe or an overlay (201..204). Drives tools/match/match_fn.py in a write-C / compare / fix loop and commits only on exit 0.
---

# match-function

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

## Shared types and names

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

## Quick reference

- MATCH / MISMATCH / ERROR line formats and exit codes: `tools/match/README.md`.
- `--json -` prints the facts (`masked_words`, `first_diff_offset`, ...) for a
  loop that wants to parse instead of read.
- Worked example that matches: `tools/match/tests/func_8004F7E8.c`; its deliberate
  mismatch: `func_8004F7E8_perturbed.c`.
