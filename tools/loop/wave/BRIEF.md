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

## When the levers run out: ask the what-if compiler

`python tools/loop/whatif.py search <unit> <vaddr> <file> [--rodata VA]` compiles your C
with a cc1 that can flip single decisions: a pseudo's allocation priority, a tie, a cse
fold, an insn's schedule slot, a loop hoist, cross-jumping. It tells you which flips bring
the function closer, for example "local-alloc does not tie pseudo 87; sched2 priority of
insn 142 +15". Read the `-dl` / `-dp` dumps to see which variable and statement those are,
then write C that provokes the same decision. It runs a few hundred compiles: one search
at a time, `-j 2`.

## Levers that matched in the tail (newest first)

- **Unaligned copies are struct assignments (wave 11).** `lwl`/`lwr` loads then
  `swl`/`swr` stores (`lwl v0,3(x); lwr v0,0(x); ...; swl v0,3(y); swr v0,0(y)`) are
  GCC's block move for a 2-aligned struct. Write `*dst = *src;` on an SVECTOR (8 bytes,
  two words) or a DVECTOR (4 bytes, one word), cast if you must:
  `*(SVECTOR *)((u8 *)p + 0x10) = *(SVECTOR *)&q->pos;`. Drafts made since wave 11 do
  this already. An older draft with `M2C_ERROR(/* ... lwr ... */)` or `M2C_UNALIGNED32`
  needs it by hand.
- **GTE code: use `#include "ac1/gte.h"` and `python tools/loop/gtemap.py <unit> <vaddr>`.**
  m2c prints every GTE instruction as `M2C_ERROR(/* unknown instruction */)`. gtemap
  names the gte.h macro for each GTE run in the retail asm, with the register that
  carries each argument (`8008cd44 gte_SetRotMatrix($10)`), so you can place the
  calls. Each macro is one asm block, so cc1 schedules C code around it but never into
  it. Match the C value that feeds the argument register.
- **One `lui`, many `reg+offset` fields: a local pointer (wave 11).** Retail reaching
  the scratchpad (0x1F800000) or a global block as `t2+offset` after a single `lui` came
  from `T *sp = (T *)0x1F800000;`. A cast macro gives every access its own `lui`.
- **Loops over records are pointer walks.** `o++` / `w++` over a struct array lets
  loop.c strength-reduce every field access into one giv; an index `a[i]` did not.
- **`x * 28` written as shifts.** `sll 5; addu; sll 2; subu` is the source's
  `(x<<5) - (x<<2)`. GCC's own multiply by a 0x1C stride gives `sll 3; subu; sll 2`.
- **A count-down loop needs at most one store.** loop.c turns a loop into a count-down
  loop only if the body has at most one memory store (`num_mem_sets <= 1` in
  check_dbra_loop). With two stores, write `n = 256; do {...} while (--n != 0);` and
  index with a separate `j++`.
- **Keep a constant in the inner preheader.** Each movable already moved doubles
  insn_count for the later ones, so a constant shared by inner loops (`-1`) gets hoisted
  to the outer loop. `neg = -1;` in the arm that uses it, as a user variable used in more
  than one block, keeps it inside.
- **Switch cases that stay separate.** When every case ends in the same call, write the
  call in each case. `k = N; break;` before one shared call lets jump2 cross-jump two
  cases that differ only in an early constant. The same goes for if/else arms: a call
  written in both arms keeps its argument setup inside the arms.
- **Register updates around calls.** sched1 moves plain updates (`j++; i--;`) across a
  call's argument setup. Write them after the call to land before the jal, as in retail.
  A `y = 130;` written after a call can fill its delay slot.
- **A counter loaded straight into a saved register** (`lbu s0; addiu s0,s0,-1`) is
  `i = p->n; i--;`. Written as `i = p->n - 1` it cannot tie.
- **t0 instead of v0.** One variable reused for two unrelated temps, neither of which
  crosses a call, becomes global-allocated and gets t0.
- **Compare operand order picks the load order.** `top <= y` and `y >= top` put the two
  sign-extending loads in opposite orders: combine merges each load into its extension
  at the compare. whatif has no knob for this.
- **Spill choice by refs.** `landed = top >= y - h` counts the scc expansion's two sets.
  `ok = ...; landed = ok;` cuts the refs, so landed becomes the spilled pseudo.
- **Operand order of `addu` with a spilled base.** `(T *)(idx * sizeof(T) + (u32)base)`
  gives `addu d, prod, base`; `&base[idx]` gives the reverse.

- **Free refs (wave 10).** flow counts reg_n_refs before combine, and combine never lowers
  the count for uses it deletes. A redundant self-cast right after the set,
  `v = (s16)x; v = (s16)v;`, raises v's allocation priority by a floor_log2 step with no
  code. It only works if combine can prove the cast redundant (a narrow load, or an
  already-extended value).
- **Free live length, placed.** A `(u16)`/`(u8)` self-cast that combine deletes leaves a
  code-free `(use reg)` insn, and sched1 counts it in the live length of every register
  live there. Chain it on a value whose definition comes early: the USE then sticks
  right before that definition. Use it to nudge one tie by +1 (ovl201 0x8007053C).
- **A sign-changing cast is a separate copy.** `z = (s16)u16var` is never CSE-shared with
  zero-extended uses (sign_extend and zero_extend hash differently), and combine reduces
  it to a plain move.
- **Identical arms hide a pointer from sched's alias analysis.** Set the pointer in both
  arms of `if ((s32)o < 0) {...} else {...}`, 2+ insns per arm: reg_n_sets becomes 2, so
  init_alias_analysis has no known value. jump2 merges the arms. Cost: the join label
  ends cse's extended block (ovl204 0x80061AD4).
- **A block-local `t = global + k` steers s0.** local-alloc gives it s0 first, and
  set_preference makes the global prefer s0, so other globals avoid it. This fixes
  saved-register swaps without priority changes.
- **Keep an address base-relative through cse2.** Set `bp = &G.f` after the base's first
  use, set it again inside a block the cse path skips, then read through `*bp`. cse1
  invalidates bp, cse2 keeps `off(base)`, and flow deletes the dead bp
  (ovl202 0x8008080C).
- **Stop the outer loop re-hoisting.** Assign the variable at the top of the inner body,
  so the inner loop hoists it, and assign it again, dead, after the inner loop: the
  outer loop sees two sets. A separate extern symbol for a field address
  (`extern u8 D_X[]`) keeps cse from relating it to the other fields.
- **do/while(0) weights compiler temps too.** To raise a global's priority without
  disturbing a block's locals, wrap a pure copy (`do { x = nx; } while (0);`).
- **Conflicts follow the sched1 order, and sched2 runs before jump2.** Loop-note barriers
  and the no-op moves jump2 deletes are still present in sched2.

- **Read GCC's source for the blocking rule.** Wave 8 matched 5 of the hardest by finding
  the exact rule in cc1's source (local-alloc.c combine_regs, loop.c invariant_p, jump.c
  cross-jumping, sched.c alias analysis) and writing C that sidesteps it.
- **Identical arms on a free test.** `if ((s32)p < 0) f(...); else f(...);` makes the
  variables multi-block, so local-alloc cannot tie them. jump2 then merges the arms and
  the branch disappears. `if (z)` does not work: the else arm folds z = 0.
- **Dead store in a loop exit path.** `rk = 0;` makes rk non-invariant for loop.c, so
  only the index is strength-reduced and `rk + off` stays an add at each use. Flow
  deletes the store later.
- **A volatile store/load pair orders just those two accesses in both sched passes.** It
  is a lighter barrier than do/while(0), which fences everything.
- **Store in each arm, then goto.** If retail branches to a shared store with the
  constant in the delay slot, write `o->f = N; goto L;` in each arm and let
  cross-jumping merge the stores.
- **A variable that dies twice is global-allocated.** Reuse a load variable for a later
  temp (`q = dir.vx; ...; q = d >> 12;`) so it cannot tie in local-alloc.
- **Cross-jumping needs at least 2 matching insns before two jumps to one label.** To
  keep two identical tails apart, make them jump to different labels.
- **Global moved behind a pointer.** If a near-copy is longer and its extra words are
  `lui`/`lw X` loads before field accesses, a block of absolute globals became
  `ptr->field` there (ovl202 reads 0x80040FA0 directly; ovl203/204 go through
  D_80039D18). Rewrite the globals as `*(T *)(ptr + off)`. If the pointer load then
  schedules too late, take the pointer into a local first.
- **Jump-table padding.** When the table offsets in `lw ...(at)` differ by multiples of
  4, retail kept cc1's `.align 3` padding between tables: try `--no-rodata-align-fix`.
- **Rewrite m2c gotos as real structure.** For a big switch-heavy function, write every
  arm out in full as switch/if code and let cross-jumping recreate the shared tails.
  This took a 6.3 KB function from 1360 words off to 0, in 46 compiles.
- **Statement after the call.** To place a register copy in the middle of a call's
  argument setup, write it *after* the call; inside an argument it changes nothing.
- **Reuse a multi-block variable.** When a temp ties to its input in local-alloc and
  retail's doesn't, reuse a variable already assigned in another block. It becomes
  global-allocated and cannot tie.
- **Port by delta.** When a twin replay misses only in lui/addiu immediates, subtract the
  two values. A constant delta points to one struct field whose offset differs between
  overlays (DrawWork.vtx sits at +0xB24 in ovl202 and +0x674 in ovl203/204). Fix that pad.
- **Bitfields.** A bitfield struct (`u32 lo:17, f:10, hi:5`) reproduces a mask/shift/or
  sequence and the scheduling around it.
- **Volatile casts.** A volatile cast on a load after a branch
  (`*(volatile u16 *)&p->f`) stops reorg filling the branch's delay slot from below. A
  volatile global does the same for a call's delay slot.
- **Switch defaults.** `case 0: case 1:` written before `default:` makes the default arm
  re-read a field instead of reusing a CSE'd value.
- **goto loops.** A loop built from `goto` is invisible to loop.c, so address loads and
  field re-reads stay inside it.
- **Dead pointer step.** `p -= 8;` after a counted loop keeps a field address in a saved
  register, so nearby fields are reached as reg+offset, not absolute.
- **do/while(0) as a barrier.** It is also a scheduling barrier: loads after it stay
  after the stores before it.
- **Per-arm stores.** Volatile stores written in each case before a `goto common` let
  cross-jumping merge them.
- **Frame padding.** A spare `u8 spare[1]` local fixes the frame of a leaf function.
- **Callee prototypes.** Check the callee's matched prototype before trusting m2c's
  argument count; m2c drops and invents arguments.
- **Last resort, and flag it in your report:** `register T v asm("$N")` pins and empty
  `__asm__` statements (an opaque pointer, a memory clobber). They byte-match, but they
  are not how the original source was written, so such a match is recorded as
  **provisional** (re/match/provisional.tsv) and gets reworked later. Try every other
  lever first. A gp base pin (`asm("$28")`) for small-data globals is fine.
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
