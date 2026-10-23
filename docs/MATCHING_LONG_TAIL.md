# Matching the long tail: why the last functions are the hardest

A field report from byte-matching Armored Core (SLUS-01323), written at
5549 / 5771 functions (96.2%) for anyone starting a matching decomp of their own.
None of this is new to compiler people. It is written down because the trends are
easy to feel and hard to plan for until you have numbers.

"Matched" here means one thing: C that the retail compiler (GCC 2.7.2 + ASPSX 2.56,
pinned in `tools/match`) turns into the exact retail bytes, judged by
`tools/match/match_fn.py`. Anything that is not a clean MATCH counts as open.

## 1. The curve

### Difficulty grows steeply with function size

Share of each size class still open, after three waves of agents, the sweep, the
permuter and twin replay:

| Body size | Matched | Open | Open rate |
|---|---:|---:|---:|
| up to 64 B | 1177 | 1 | 0.1% |
| 65-128 B | 1212 | 4 | 0.3% |
| 129-256 B | 1077 | 15 | 1.4% |
| 257-512 B | 928 | 35 | 3.6% |
| 513 B - 1 KB | 852 | 71 | 7.7% |
| 1-2 KB | 220 | 63 | 22% |
| 2-4 KB | 46 | 24 | 34% |
| over 4 KB | 30 | 16 | 35% |

(Counts taken just before this change; it moved seven of the open rows.)

The median matched function is 160 B. The median open function is 920 B. Size alone
ranks a random open function above a random matched one 87% of the time (AUC 0.87).

### The work drifts toward the big bodies

Bodies matched each day (originals, not copies credited by twin replay), from
`tools/loop/ledger.tsv`:

| Day | Bodies | Median size | Over 1 KB |
|---|---:|---:|---:|
| 2026-10-19 | 103 | 156 B | 4 |
| 2026-10-20 | 571 | 124 B | 14 |
| 2026-10-21 | 800 | 176 B | 7 |
| 2026-10-22 | 605 | 524 B | 103 |

Count is a misleading progress measure late in a project. On the last day the
function count barely moved compared with the bytes matched. Track bytes as well as
functions, and budget the tail by bytes.

### Half the remaining code is in a handful of functions

The 229 open functions (before this change) are only 148 distinct bodies: overlays
202, 203 and 204 carry near-identical copies of the same code. Nine bodies over 4 KB
hold 113 KB, a third of the remaining bytes. The largest is a single 25.5 KB
function.

## 2. What does *not* make a function hard

To compare fairly, the matched functions were reweighted to the open set's size mix.
On that basis, the features people usually blame are not over-represented in the
open set:

| Feature | Open | Matched, same sizes |
|---|---:|---:|
| GTE (3D coprocessor) instructions | 14% | 14% |
| `switch` jump tables | 17% | 21% |
| Calls through function pointers | 8% | 20% |
| Unaligned copies (`lwl`/`swl`) | 29% | 28% |

The density of calls, loads, stores and library calls is about the same as well.
Exotic-looking code is not what is left. Once someone has worked out an idiom (GTE
inline macros, jump tables, struct copies), it is solved everywhere.

## 3. What does

After controlling for size, these push a function toward staying open (adding them
to size raises the AUC from 0.87 to 0.93):

| Feature | Open | Matched, same sizes |
|---|---:|---:|
| Three or more loops | 27% | 8% |
| Stack frame of 256 B or more | 8.7% | 0.4% |
| All nine callee-saved registers in use (`s0`-`s7` and `s8`) | 20% | 11% |

Location matters too. Among 256 B - 4 KB bodies, 14-17% of ovl202-204 (the in-game
engine: rendering, physics, targeting) is still open, against 7% of ovl201 (menus,
garage) and 4.6% of the mission overlays.

All three features describe the same thing: many values alive at once, competing for
too few registers. That is where an old compiler's decisions become fragile.

### Why register pressure makes matching chaotic

GCC 2.7.2 assigns registers by a priority score. Roughly, that score is how often a
value is used divided by how long it stays live, and ties go to whichever value was
created first. In a short function there are few candidates and the outcome is
stable. In a long, loop-heavy function, a dozen values have nearly equal scores for
nine callee-saved registers. Then a change that does not alter what the code does
(declaration order, an extra temporary, where an assignment sits) reorders the ties
and reshuffles the whole assignment.

A mechanical analogy: a slender column under load. Below the critical load, small
imperfections barely matter. Near it, a tiny imperfection decides which way the
column buckles. A short function is the stubby column; a 2 KB render loop is the
slender one.

Three consequences:

1. **Progress is not smooth.** A short function improves step by step as the C gets
   closer. A long one jumps between distant outcomes. Reasoning stops helping, and
   search (the permuter) is the right tool.
2. **The diff count lies.** One swapped register shifts every following instruction,
   so "326 of 329 words differ" can be a single decision. One agent's "548 words off"
   was about 13 real differences. Measure closeness by structure (same size, same
   control flow, same frame), not by differing words.
3. **Other passes inherit the noise.** The stack frame size follows from how many
   values spill to the stack, which follows from register allocation. Instruction
   scheduling and the loop optimizer (strength reduction, hoisting) make similar
   threshold decisions.

### What the near misses were actually stuck on

The agents left a note for each of 104 near misses. Sorted by family (a note can name
more than one):

| Blocker | Notes |
|---|---:|
| Register allocation (which variable lands in which register) | 63 |
| Instruction scheduling (a load or store one slot earlier or later) | 38 |
| Constant reuse and addressing form (base register vs. direct address) | 20 |
| Loop optimizer (strength reduction, invariant hoisting, block moves) | 18 |
| Block layout and cross-jumping (shared tails merged one way or the other) | 10 |
| Stack frame size (one or two extra spill slots in retail) | 10 |
| Toolchain or tooling | 3 |

24 notes are register allocation and nothing else: the C is right, only the register
numbering differs.

## 4. Hardness that is not about the code

Some open functions were not hard at all. They were stuck on something around the
code. In order of how often we hit it:

- **Copies with different addresses.** Overlays 202-204 duplicate code, and the
  copies refer to the same globals at different addresses. Our replay tool
  (`tools/loop/twins.py replay`) moved a symbol only when a relocated address lay
  within 4 KB above it, so struct bases reached through far fields (one at
  +0x18000) stayed put, and could pick up a neighbouring variable's shift. Seven
  copies sat open for that reason until this change added a wider retry.
- **Tool gaps.** One jump-table body needed an oracle flag (`--no-rodata-align-fix`)
  that the driver could not pass through. One flag was the whole fix.
- **Bugs in the rebuilt toolchain.** cc1 from 1995 assumes a 32-bit host. Built as a
  64-bit Windows program, functions used without a declaration return pointers
  truncated to 32 bits. This crashed or miscompiled two classes of input (float
  constants, and a delay-slot pass under address-space randomization). Each fix
  unblocked functions that had looked "too hard".
- **Wrong function boundaries.** One open body is a mis-split in the function
  inventory: its real start is 20 bytes later. No C can match a function that does
  not exist.
- **Not compiler output at all.** Two functions are hand-written assembly in Sony's
  PsyQ MDEC library. They are recorded as give-ups, with the reason, so no tool or
  agent spends time on them again.

## 5. Advice for the next project

1. **Build the judge first, and trust nothing else.** One command that says MATCH or
   not, exactly as the retail build would. Every tool, agent and human checks against
   it, and no "looks right" counts.
2. **Expect a long tail and budget for it.** Here the last 8% of functions took a
   full day of a dozen parallel agents plus overnight machine passes, and still
   left 4% open. Plan in bytes, not function counts, and expect each of the final
   large bodies to cost as much as dozens of small ones.
3. **Harvest duplication early.** Find identical or near-identical bodies (masked for
   relocations) before anything else. One match can credit dozens of copies; that is
   how 3,600 of our matches came about.
4. **Suspect the toolchain before the code.** An old compiler rebuilt for a modern
   host is a new program. Run it with a reproducible environment, check its build
   for undeclared functions returning pointers, and turn off address-space
   randomization. When "correct" C will not match, rebuild the compiler under a
   debugger before rewriting the C again.
5. **Split "wrong C" from "right C, wrong registers".** When size, control flow and
   frame match, stop reasoning and start searching. The permuter tries thousands of
   neutral rewrites per minute; a person or an LLM tries a few dozen.
6. **Pick the right tier for each kind of difference.** Here, deterministic rewrites
   (put retail's constant in, flip a comparison) caught everything a small local
   LLM caught, for free. Frontier-model agents earned their cost on structure:
   struct layouts, control flow, idioms. The permuter earned its cost on registers.
7. **Make every near miss leave a note.** One line of "what is still different"
   per attempt. Those notes are the dataset this report is built from, and they let
   the next attempt start where the last one stopped.
8. **Decide early what you will not count.** Hand-written assembly and library code
   can never be matched from C. Decide up front how to count them, and apply the
   rule to whole libraries rather than to whichever functions are left open.

## 6. Postscript: the permuter pass

After this report was first written, the permuter (`tools/loop/permute.py run`:
deterministic levers, then decomp-permuter, 20 minutes per body) was run on the
wave-3 agents' best C for 76 of the near misses. It matched 11 of them. By how far
each body started from a match:

| Words off at the start | Tried | Matched |
|---|---:|---:|
| 5 or fewer | 12 | 4 |
| 6-20 | 24 | 6 |
| 21-60 | 16 | 1 |
| more than 60 | 22 | 0 |

Two lessons:

- **"Close" is not the same as "almost done".** Eight bodies that started 2-5 words
  off did not match in 20 minutes of random rewrites. A register-allocation tie is
  a narrow target, and most rewrites move the function away from it, not toward
  it. Bodies more than 60 words off never matched: there, the C is still
  structurally different, and that needs a person (or an agent), not search.
- **Search tools are heavy, so budget the machine.** Each permuter job holds a
  copy of the compiler pipeline and a growing candidate cache. 20 parallel jobs on
  2-3 KB bodies exhausted Windows system resources, and a rerun at 12 jobs ran
  out of memory and took the PC down with it. On a desktop someone is also using,
  run 2-4 jobs and let it take longer.

The 31 largest near misses (1.5-3.4 KB) were not reached and are left for a
slower pass.

## Method

Features were computed from the retail instruction words of every non-PsyQ
function in `tools/loop/functions.tsv`: size, loops (backward branches), stack
frame, saved registers, and instruction mix. The open/matched split comes from
`re/match/manifest.tsv`. Blocker families come from keyword tags over the wave-3
agents' near-miss notes. The size-controlled comparisons reweight matched functions
to the open set's log2 size distribution; the AUC figures come from a logistic
regression on standardized features.
