# Tool router for hard functions — scoping brief

Status: scoped 2026-10-22; built and evaluated the same day (results at the end).

## Problem

Claude usage is the bottleneck: we have hit the usage limit many times this week.
Much of it goes on large functions that a cheaper tier (the permuter, the
`permute.py` levers, or a golem) could have closed. We want a **router** that
looks at an open function and picks the cheapest tool likely to close it, so
Claude only gets the functions that genuinely need it.

## Question to answer

On held-out matched functions, how often does the router pick the tool that
actually closed the function, and how much Claude usage would it have saved?
Fine-tuning a golem is only worth trying if routing alone leaves a big gap.

## Inputs that already exist (check before rebuilding)

- `tools/loop/functions.tsv`: static features per function (size, instructions,
  branches, calls, jump tables, GTE ops, stack size).
- `tools/loop/ledger.tsv` + `re/match/<unit>/*.attempts` sidecars: who or what
  closed each match, and how many tries it took.
- `tools/loop/closeness.tsv`: how close each open function's current C is.
- `re/match/manifest.tsv` and `twin_replay.tsv`: the matched set. Twin rows must
  be excluded or grouped, so a clone can't leak between train and test.
- `tools/loop/sweep.py` (zero-agent m2c + levers), `permute.py`, `levers.py`,
  `golem.py`, `hints.py`: the tiers being routed between.
- `tools/loop/rank.py` (`--calibrate`) and `refit.py`: an existing difficulty
  score over `functions.tsv` columns, with a `log(attempts) ~ features` fit over
  ledger outcomes. The router should extend this, not duplicate it;
  `make_queues.py` already bands the queue by that score.
- The golem bake-off session (PR byroweb/AC1_RE#84, branch
  `claude/adoring-bouman-c71799`): a feature table for the 229 open functions
  (148 distinct bodies), the finding that register-allocation ties dominate,
  and per-step usage estimates. Only `golem.py`, its README section, the test
  and 3 matches were committed; the feature table lives in that session's
  scratchpad, so read it from there or ask that session.
- The open set as of 2026-10-22: 228 non-PsyQ functions with no ledger row
  (ovl201 31, ovl202 51, ovl203 44, ovl204 41, exe 5, fdat 56).
  `closeness.tsv` covers 1236 open *bodies* from the 2026-10-20 sweep, a wider
  set; say which set a number refers to.

## Labels (the first real task)

For each matched function, derive which tier **closed** it:

1. levers / `permute.py` alone (no Claude);
2. decomp-permuter (overnight run);
3. golem (when GOLEM=1 was on);
4. Claude, short (≤ N tries);
5. Claude, long (> N tries);
6. still open / given up (a censored label — keep it, don't drop it).

What the ledger actually holds (checked 2026-10-22): no tier column, only
`status`, `attempts` and a free-text `reason`. Of the 5695 match rows, 486 say
`zero-agent sweep (m2c ctx)` (tier 1), about 3760 are `twin of ...` rows
(derived, not an outcome: exclude), and 1449 have an empty reason (a Claude
match; `attempts` is the oracle-run count, 0 = unknown). Only 21 `.attempts`
sidecars exist, and the commit log names the permuter in 10 commits and the
golem in 1, so tiers 2 and 3 have at most tens of examples. Reconstruct what
the commit messages and sidecars allow and record the rest as unknown; don't
guess. Expect the learnable label to be "sweep/levers close it" vs "Claude,
short" vs "Claude, long"; whether the permuter or golem would have closed a
function must come from running those tiers on held-out open functions, not
from labels.

## Features

- Start with `functions.tsv` columns plus cheap derived ones: loop depth,
  callee-saved register pressure (the count of `s0–s7` used), basic blocks,
  switch vs. if-tree shape, mission template family (`tools/archaeology`).
- Add **diff features** where a first m2c draft exists: number of differing
  words, whether the diff is only register names (a register-tie signature) or
  also opcodes and branches (a structural miss).

## Models, in order of cost

1. A rule baseline: e.g. a register-only diff goes to the permuter, a structural
   diff to Claude. Diff features need a compiled first draft, so they are
   computed where the toolchain is (this PC); the static features and labels are
   pure TSV work and can run anywhere.
2. A decision tree or gradient boosting over the features. Interpretable; train
   on CPU.
3. Only if 1–2 plateau: a local golem as a classifier (prompt or a small LoRA) on
   the asm and diff summary. It predicts a tier, not C.

## Evaluation

- Split by **body**, not by function, so twins never straddle train and test.
  Hold out about 20 %, stratified by size bucket. Report the big-function bucket
  on its own.
- Metrics: tier accuracy; the "regret" (the extra Claude windows spent when the
  router picks too expensive a tier, and the wasted CPU when it picks too cheap);
  and the projected Claude windows saved on the 229 open functions.

## Deliverables

- `tools/loop/router.py`: `router.py rank` writes a queue TSV with the predicted
  tier per open function; `router.py eval` prints the metrics.
- A labels TSV and a short results section appended to this doc.
- A recommendation: wire the router into `overnight.sh` and the queue builder
  (`make_queues.py`), or not. Never edit `overnight.sh` mid-run.

## Out of scope

- Fine-tuning a golem to write C (revisit only if the results justify it).
- Block-region splitting of huge functions. Note it as a follow-up if the router
  shows many functions with structural misses too big for one Claude pass.

## Results (2026-10-22, `tools/loop/router.py`)

Status: built and evaluated; not wired into `overnight.sh` or `make_queues.py`
(recommendation below).  Everything is pure Python over committed TSVs:

```bash
python tools/loop/router.py labels      # tools/loop/router_labels.tsv from the ledger + git + sidecars
python tools/loop/router.py features    # tools/loop/router_features.tsv (needs the images; committed)
python tools/loop/router.py eval        # ~90 s: holdout + 5-fold grouped CV, regret, open-set projection
python tools/loop/router.py rank --report WORK/sweep/sweep_report.tsv \
    --permute-report WORK/perm1/permute_report.tsv --permute-report WORK/perm2/permute_report.tsv
                                        # tools/loop/queue-router.tsv: tier to try next per open function
```

### Labels: what the ledger actually supports

`router_labels.tsv`, one row per ledger key (newest row wins), `label_source` on
every row.  Of 5547 keys, 3540 are `twin of` rows (derived; tier `twin`, never
trained on).  The 2006 outcomes with features:

| tier | closer | n | how it was derived |
|---|---|---:|---|
| 1 | sweep | 529 | reason `zero-agent sweep (...)` (all variants, not only `m2c ctx`) |
| 1 | levers | 57 | reason `permuter (levers ...)`: `levers.py` alone, no permuter run |
| 2 | decomp-permuter | 40 | reason `permuter (decomp-permuter ...)` |
| 3 | golem | 3 | reason `golem (...)` |
| 4 | Claude, at most 2 tries | 549 | `attempts`, the `matched func_<ADDR> N` commit subject (the larger of the two: for 210 lane rows the ledger says 1 and the commit 2 to 12), `.attempts` sidecars (21) |
| 5 | Claude, more than 2 tries | 245 | same |
| 4/5 | Claude, tries unknown | 580 | rows added by `matched N ... functions` / wave-2/3 batch promotes: one re-verifying `drive.py try`, so `attempts` = 1 says nothing; 4 seeded from the manifest |
| 6 | give-up | 3 | ledger status |

So the ledger is richer than the brief assumed (100 permuter/levers rows, not
"tens"), but the Claude try count is only known for 794 of 1374 Claude rows, and
tiers 2 and 3 together have 43 examples.  `--short-max` sets the short/long cut
(default 2: a first draft plus one fix).

One more censoring: the 110 Claude rows matched before 2026-10-20 never had a sweep
run on them (there was no sweep yet), so their tier-1 outcome is unknown; the
static tasks leave them out (`--claude-since ''` keeps them).

### Features

`router_features.tsv`, one row per non-PsyQ function (5771): the `functions.tsv`
columns and `rank.score()`; from the image words: loops (backward branches), loop
depth and span, basic blocks and the largest block, conditional vs unconditional
branches, `jr` tables, `jalr`, returns, callee-saved registers saved (`s0` to `s7`,
`s8`), frame size, sp-relative traffic outside the prologue (spills), mul/div,
byte/half loads, sign-extends, compares, unaligned loads, `lui`, loads/stores, `gp`
refs, PsyQ calls, distinct callees, nops, GTE pairs; the twin group and its size;
the coarse-skeleton family (every immediate masked, the identity
`tools/archaeology/missions.py` uses: the same source function edited between
missions or overlays) as `family_size`, `family_units`, `family_extra` (members
beyond exact twins); and the best m2c draft's `closeness`, `positional`, their
difference (`shift`: a size change or block move), the estimated differing words
and the variant class, from `closeness.tsv` (2026-10-20) with twins of a swept body
inheriting its row.  1991 functions have a draft closeness.

**Diff hook, not built.** The register-only vs structural miss feature needs the
compiled draft's words next to the retail words.  `router.py features --diff F`
merges a table `unit vaddr words reg_only size_delta` into the `diff_words_real` /
`reg_only` / `size_delta` columns, and the learners use them when present, but
nothing produces that table yet: for the open set it would be a short script over
`sweep.py --best-dir` output and `match_fn.py --keep` objects (this PC, minutes);
for the *training* rows the historical drafts are gone, so the feature could only
be evaluated on functions matched from now on.  The closest available proxy, the
differing-word count in the wave-3 agents' near-miss notes
(`tools/loop/near_misses.tsv`, 96 open functions), is used by `rank`, not by the
models.

### Evaluation

Split by body (twins.py groups), 20 % held out stratified by size bucket
(small under 256 B, mid under 1 KB, big 1 KB and up), plus 5-fold grouped
cross-validation (every function held out once).  Learners: the rule baseline
(thresholds grid-fit on the training rows), a CART tree (depth 5) and a boosted
ensemble (60 rounds, depth 3), all pure Python.  Headline numbers are the 5-fold
ones; the fixed holdout agrees within a few points (in `eval` output).

Cost model (knobs): Claude 3 min/KB short, 8 min/KB long, floors 2/8 min (the
README calibration's 0.5 to 3 min/KB for siblings, 5 to 10 min/KB first-of-family);
permuter 7 CPU min per body (`QUICK` 120 s + `DEEP` 300 s); one usage window =
720 Claude minutes.  Check: wave-3 batch 1 (112 functions, 44 KB, 12 agents for
about one hour = one window) costs 579 model-minutes, so the window knob is within
about 20 %.

**Backends.** The numbers in this section are from the pure-Python learners
(`--backend python`, about 70 s for `eval`) and stay the reference.  When numpy and
scikit-learn import, `--backend auto` (the default) runs scikit-learn's
`HistGradientBoostingClassifier` and `DecisionTreeClassifier` with the same
rounds / depth / learning rate / leaf size instead (about 4 s for `eval`); the
output names the backend that ran.  On the same splits the 5-fold boosted
accuracies are 0.848 / 0.687 / 0.947 / 0.698 (static2 / static3 / draft2 / draft3)
against 0.851 / 0.691 / 0.946 / 0.705 pure Python, and the tree 0.803 / 0.640 /
0.939 / 0.636 against 0.809 / 0.637 / 0.929 / 0.641: within a point everywhere, the
same conclusions.  The committed `queue-router.tsv` is the pure-Python snapshot;
`rank --backend sklearn` moves 21 of the 226 open functions between Claude short and
Claude long (the weakly predictable split) and none between a machine tier and Claude.

**Task static2: before any CPU is spent, will a machine tier (sweep, levers,
permuter, golem) close it, or does it need Claude?**  1896 functions / 1885
bodies, 629 machine vs 1267 Claude, static features only.

| model | accuracy | machine F1 | Claude F1 | small / mid / big accuracy | too expensive (closable sent to Claude) | too cheap (Claude sent to a machine tier) | Claude min saved of possible |
|---|---:|---:|---:|---|---|---|---|
| rules (`instructions <= 40`, no jump table) | 0.722 | 0.64 | 0.78 | 0.59 / 0.91 / 0.99 | 169 fn, 44 KB, ~850 min | 358 fn, ~2510 CPU min | 2300 / 3149 |
| CART depth 5 | 0.809 | 0.72 | 0.85 | 0.74 / 0.91 / 0.98 | 164 fn, 31 KB | 199 fn | 2325 / 3149 |
| boosted | **0.851** | **0.78** | 0.89 | 0.79 / 0.93 / 1.00 | 137 fn, 25 KB, ~685 min | 145 fn, ~1015 CPU min | **2464 / 3149 (3.4 of 4.4 windows)** |

The big bucket (126 functions of 1 KB and up) is trivial: none of them was ever
closed by a machine tier, and every model says Claude.  All the difficulty is in
the small bucket, where the sweep closes about half.

**Task static3: the same, with Claude split into short (at most 2 tries) and
long.**  1316 functions with a known try count: 629 / 471 / 216.  Boosted:
accuracy 0.691, F1 machine 0.82, short 0.63, long 0.42; long functions routed as
short 111 of 216, short routed as long 64 of 471; big bucket (23) 0.65.  The rule
baseline (`score <= 8` = short) is at 0.53.  Short vs long is only weakly
predictable from static shape, as the README's calibration already said: the
cost is compiler behaviour (register ties, block layout), not size.

**Task draft2: after the sweep missed, with the best draft's closeness, will
levers / the permuter / the golem close it, or Claude?**  1046 functions with a
draft (78 machine vs 968 Claude).  This is the decision the overnight pass makes.

| model | accuracy | machine F1 | machine caught | false machine | Claude min saved of possible |
|---|---:|---:|---|---|---|
| rules (closeness >= 0.97 or <= 16 instr) | 0.866 | 0.26 | 25 / 78 | 87 | 125 / 390 |
| CART | 0.929 | 0.53 | 41 / 78 | 37 | 205 / 390 |
| boosted | **0.946** | **0.60** | 43 / 78 | 22 | 215 / 390 (0.3 window) |

The tree's top rule is the register-tie signature the scope asked for, read off
the closeness columns: `closeness > 0.90 and positional > 0.95 and no
unconditional jump` gives 0.89 machine (same size, same block layout, a few words
off); `closeness > 0.90 but positional <= 0.95` (a block moved or the size
changed) gives Claude.  Draft3 (short/long added, 599 rows with known tries):
boosted 0.705, machine F1 0.66, short 0.77, long 0.56.

**Honest reading.** The learnable split is "a machine tier closes it" vs "Claude",
at 0.85 before any CPU is spent and 0.95 once a draft exists; short vs long Claude
is at 0.6 to 0.7 and the long class is the weak one (F1 0.4 to 0.56).  Tiers 2 and
3 are not separable from tier 1 with 43 examples; the models treat levers,
permuter and golem as one "machine" class, and which of them closes a near miss
must still come from running them (cheap).  The "Claude minutes saved" column is
counterfactual: it is what the router recovers of the Claude time the sweep-first
policy already saves; the project already runs every machine tier on everything
before an agent sees a body, so routing alone cannot save Claude windows on bodies
the machine tiers have already failed on.

### The open set

226 non-PsyQ functions without a ledger row (146 bodies, 344 KB; ovl201 31,
ovl202 49, ovl203 44, ovl204 41, exe 5, fdat 56), ranked with the 2026-10-22
overnight pass's sweep and permuter reports and the wave-3 agents' near-miss notes
(`tools/loop/queue-router.tsv`):

| next tier | functions | KB | why |
|---|---:|---:|---|
| 1 sweep | 2 | 0.2 | no draft on file, P(machine) >= 0.5 (one is `libpress` hand asm: not our compiler, skip) |
| 2 permuter | 33 | 31 | an agent's C within 12 words (`near_misses.tsv`); the permuter has not seen these texts |
| 4 Claude short | 72 | 78 | machine tiers spent (sweep + quick/deep permuter), P(short) >= 0.5 |
| 5 Claude long | 119 | 235 | machine tiers spent; 16 are giants of 4 KB and up (112 KB) |

Projected Claude windows saved on this set by routing: about 0.05 (35 Claude
minutes).  The static model finds 5 of 226 machine-closable and the post-sweep
model 0 of the 205 that already have a draft: the open set is the residue after
sweep, levers, permuter and twelve agents, and the router correctly says so.  What
is left to save is not in routing but in the two rows above that are actionable
without Claude: the 33 agent near misses (the permuter's pilot closed 35 of 48
bodies within 2 words and 5 of 32 larger ones, so expect 10 to 15 of these,
roughly one Claude hour), and the giants, which no tier handles (block-region
splitting, the scope's listed follow-up).  Fine-tuning a golem is not justified
by these numbers: the gap is not in picking a tier.

### Recommendation: wiring

1. **Do not gate the machine tiers with the router.**  They cost CPU, which is
   free overnight, and the regret of skipping a closable body is Claude time.
   `overnight.sh` should keep sweeping everything.  Where the router pays is the
   deep permuter pass (300 s times `DEEP_N`): order its candidates by the
   post-sweep P(machine) (`draft2` model) instead of by differing words alone, so
   the budget goes to same-size / same-layout near misses first.  That is an
   `--order` option on `permute.py pick` reading `queue-router.tsv`, not an
   `overnight.sh` edit.
2. **Feed the agents' near misses to the permuter.**  `overnight.sh` only permutes
   the sweep's own drafts; the 33 tier-2 rows above are agent C files it never
   sees.  `permute.py run --cfile UNIT:FILE ...` already takes them; the one-line
   change is a `CFILES` list built from `queue-router.tsv` tier-2 rows, added
   once the current run has finished.  (A permuter pass over 108 of these was
   started on 2026-10-23 from a work dir and every job reported `crash`: that is
   the cc1 reorg/ASLR crash the wave-3 notes describe, fixed on the
   `wave3-batch2` branch, so re-run it from a checkout that has that fix.)
3. **`make_queues.py --bodies --router tools/loop/queue-router.tsv`.**  Keep
   its payoff order but (a) drop tier-1/2 rows from the agent queues (they are
   machine work), (b) put tier-4 bodies before tier-5 ones so agent lanes hit
   their limit on functions they can finish, and (c) hold tier-5 giants out of
   lanes and list them in a `queue-G.tsv` for the splitting follow-up.  This is
   a TSV join, no learner inside `make_queues.py`; the `hints.py` triage rows
   (`skip` / `priority`) keep precedence.
4. **Refresh the labels after every promote.**  `promote.py` should write the
   agent's try count (its PROGRESS.md has it as `MATCH 0`, always 0 today) into
   the ledger's `attempts`, so the 4/5-unknown class stops growing; the
   short/long models are the ones starved of labels.
5. Re-run `router.py eval` after the next 200 matches; if `draft2`'s machine F1
   stays above 0.6 and the open set stays a residue, the router has done its job
   and the next tool is block-region splitting, not a golem.
