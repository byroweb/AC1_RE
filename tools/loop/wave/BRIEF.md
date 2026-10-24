# Matching wave brief (shared by every agent)

<!-- Template: `wave.py brief OUT` fills {REPO}, {OUT} and {OPEN}. Edit the levers list as
     the project learns; the newest levers belong at the top. -->

Repo (run every command from here): {REPO}
Wave folder: {OUT}. Shell: Git Bash via the Bash tool. Python: `python`. Run
`export PYTHONIOENCODING=utf-8` once.

You byte-match open functions of Armored Core (PS1, SLUS-01323) to C that the retail
compiler (GCC 2.7.2 + ASPSX 2.56, pinned in tools/match) turns into exactly the retail
bytes. {OPEN} functions are left, and they are the hard tail. Your list is one row per
**family**: functions with the same instruction shape. Match the row's function, and the
coordinator ports the match onto the `members` with tools. Never port by hand. The
`draft` column is the best C so far, `words` is how far it is from retail, and `note` is
what the last agent found.

## Rules

Other agents work in the same checkout at the same time.

- **Where you write.** Write files ONLY under your SCRATCH dir (given in your prompt). C
  goes to SCRATCH/<unit>/func_<ADDR>.c. Never edit, create or delete files in the repo.
  Never commit, and never run git commands that change state.
- **The judge.** `python tools/loop/drive.py try <unit> <vaddr> --re-dir SCRATCH [opts]`,
  where `opts` is the row's `opts` column. Only exit 0 ("clean MATCH") counts.
- **CPU: one compile at a time.** Use `variants.py ... -j 1` only. `regtie.py run` and
  `structdiff.py --fix` are allowed one at a time with `--evals 300` or less. Never run
  the permuter (`permute.py run`) or anything with -j above 1.
- **Stop time.** If your prompt gives a stop time, check `date` between rows. Stop at
  that time, even mid-list, and report. Usage is shared and must not hit its cap.
- **Read first:** `.claude/skills/match-function/SKILL.md` (especially "Stuck on a near
  miss? Ask the doctor"), `include/ac1/README.md` and `docs/MATCHING_GUIDE.md`. Use
  `#include "ac1/ac1.h"`. Declare missing struct fields or names locally in your C file;
  don't edit shared headers, but list proposals in your report.
- **Skip what's done.** If `re/match/<unit>/func_<ADDR>.c` exists without a "NOT
  MATCHED" line, the function is done: skip it.

## How to work a row (in this order)

1. **Ports first.** If the note says `PORT from matched X`, start from X's C in re/match.
   Rename its symbols to this unit's addresses, then find the few numbers that differ:
   compare the two functions' retail words. They are usually a struct pad or offset, a
   local array size (frame), or a screen limit.
2. Copy the `draft` file into SCRATCH/<unit>/func_<ADDR>.c and run `drive.py try` once.
3. Ask the doctor: it compiles once and says what kind of miss this is.
   `python tools/loop/doctor.py check <unit> <vaddr> SCRATCH/<unit>/func_<ADDR>.c[@rodata=VA][@noalign]`
4. **registers / mixed / size-small:** `python tools/loop/regtie.py diagnose <unit> <vaddr> <file>`
   names the variable in the wrong register, who got that register instead, and both
   allocation priorities (floor_log2(refs) * refs / live_length; the higher one is
   allocated first). Change the C so the priorities flip.
   `regtie.py run --cfile <unit>:<file> --evals 300 --dry-run` searches edits for you.
5. **structure / size:** `python tools/loop/structdiff.py <unit> <vaddr> <file>` prints
   the first divergence side by side, with C line numbers and named findings: loop
   test at top or bottom, jump table vs compares, cross-jumped tail, swapped arms,
   frame size.
6. Read the diff yourself. The tools tell you *where*; you supply the *what*.

## Levers that matched in the tail (newest first)

- `do { stmt; } while (0)` around a statement raises its variables' allocation
  priority, because loop depth weights their refs. This flipped three register ties.
- A delay slot filled from the branch *target* means retail predicted the branch taken.
  Put a do/while(0) label block at the target.
- `k = n = f(...)` keeps k as the canonical copy, so GCC does not propagate a copy
  backwards.
- A flag computed before the decrement (`x = k < 0x80; k -= 0x80;`).
- A field read through a cast pointer (`*(s16 *)((u8 *)w + 0x16)`) is not a struct
  access, so its load stays ordered after a volatile store.
- Count loops written forward (`i = 0..N`), or as `i = n; while (--i != -1)`.
- An invariant base pointer plus an index (`q[j*30]`) instead of a stepped pointer.
- A narrow (u8) temp for a select value keeps CSE from merging it with a compare constant.
- Declaring a local in an inner block gives it a later pseudo number, so it loses ties.
- `(s16)(x - 7000) + r` keeps the constant on x; plain `x - 7000 + r` reassociates.
- A sum that needs its own temp: `{ u8 *b = p + 0x6800; src = (u32 *)b + n; }`.
- Split a variable reused for unrelated values, or reuse the tied partner's variable.
- Store order inside a block follows the source order. Retail order is often the
  struct-field order.
- Cross-jumping: write a tail out in every path (or `goto` an earlier `return 0`) to
  choose which tail merges.
- Older ones:
  - struct-member globals (`extern struct {T v;} X;`) for base-register loads;
  - a spare `SVECTOR` or `char pad[N]` local for frame size;
  - a varargs handler for the a1-a3 save prologue;
  - case-body order sets the compare tree;
  - the `abs()` builtin;
  - row-struct arrays for index order;
  - `while` vs `if` + `do-while` for hoisted constants.

## Budget and order

Go down your list in order. Spend about 20 oracle tries per row (40 for 1 KB and up),
then keep your best C (note its words) and move on. A 2-10 word miss deserves the full
budget; give a 150+ word miss one honest look. Give every row a go before going deep
on any one.

## Record as you go (restarts depend on it)

- **SCRATCH/PROGRESS.md:** one line per row, `unit vaddr STATUS words opts note`, with
  STATUS MATCH / NEAR / SKIP. On MATCH, copy the `commit:` line drive.py printed (its
  --defsym / --rodata flags) into the note: `wave.py items` reads it.
- **SCRATCH/names.tsv:** `unit<TAB>vaddr<TAB>name<TAB>evidence` (snake_case with a module
  prefix; `-` when unsure).
- **Restarts:** if SCRATCH already has files, read PROGRESS.md first and continue where
  it stopped.

## Report (your final message, under 40 lines)

1. **MATCHED:** `unit vaddr bytes SCRATCH/<unit>/func_<ADDR>.c` plus the commit flags.
2. **NEAR:** `unit vaddr words` and what blocks it.
3. **Proposals:** header proposals and new levers.
4. **Leftovers:** how many rows you did not reach.
