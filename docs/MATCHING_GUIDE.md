# Matching decompilation: start here

This page is the way in to the matching work: what the words mean, which tool to reach
for, and the rules that keep the machine and the shared usage plan safe. The tools
themselves are documented in `tools/loop/README.md` (one section per tool) and
`tools/match/README.md` (the oracle).

## The goal in one paragraph

We rewrite the game's machine code as C that the original compiler (PsyQ's GCC 2.7.2
plus the ASPSX assembler) turns back into **exactly** the same bytes. A function counts
as matched only when the oracle says so. Progress is matched functions out of all
non-library functions in the exe, the four program overlays (ovl201-204) and the 58
mission overlays (fdatNNN). `python tools/loop/progress.py` prints the current count.

## Glossary

| Term | Meaning |
|---|---|
| **oracle** | `tools/match/match_fn.py`: compiles one C function with the retail toolchain and compares bytes. Exit 0 with a final `MATCH` line is the only judge. `drive.py try` is the everyday wrapper. |
| **match / clean MATCH** | Every byte equal, nothing masked, right size. Recorded as a row in `re/match/manifest.tsv` and the ledger. |
| **near miss** | C that compiles to the right shape but differs in a few instruction words. |
| **words off** | How many 4-byte instruction words differ from retail. The usual distance measure: 2 words off is close, 150 is a different shape. |
| **unit** | One code image: `exe`, `ovl201`-`ovl204` (program overlays), `fdatNNN` (mission overlays). |
| **overlay** | Code the game loads into memory only for one mode or mission. The same source was often compiled into several overlays. |
| **twin** | A function that appears in several overlays, identical apart from addresses. `twins.py` groups them. One match is **replayed** onto every twin for free. |
| **near-copy** | A twin that also differs in a number or two: a struct field offset, a local array size, a screen limit. Twin groups miss these; `nearport.py` finds them. |
| **shape** | A function's sequence of instruction types, ignoring registers and numbers. Near-copies share a shape. |
| **family** | Open functions with the same shape (or in one twin group). **The unit of work**: one family per agent; tools port the match to the rest. |
| **body** | One distinct function source. A body with 3 copies is worth 3 functions. |
| **draft** | A C attempt that is not yet a match. The best one per function is kept and handed on. |
| **lever** | A known C rewrite that nudges the compiler a specific way without changing meaning: reorder stores, split a variable, `do { } while (0)`, etc. `BRIEF.md` lists the ones that worked. |
| **register tie** | Two variables with nearly equal allocation priority, where retail gave the register to the other one. Fixed by changing refs or live ranges, not by guessing. |
| **structural miss** | The control flow differs: a loop tested at the top vs the bottom, a jump table vs compares, a merged ("cross-jumped") tail, swapped if/else arms. |
| **promote** | Copy an agent's matched C from its scratch folder into `re/match`, re-run the oracle there, commit. |
| **provisional** | A match whose C forces the compiler's hand with `register ... asm("$N")` pins or empty `__asm__` statements. The bytes are verified, but it is probably not the original source. Listed in `re/match/provisional.tsv` and shown apart by `progress.py`; to be rewritten cleanly later. |
| **give-up** | A function recorded as not matchable for now (`drive.py giveup`), for example hand-written assembly. |
| **tier** | One of the escalating ways to close a function: sweep (no AI), permuter (random search), golem (local model), compiler-reading tools, agents. |
| **wave** | A batch of matching agents working in parallel, each with its own list. See `tools/loop/wave/README.md`. |
| **gate / headroom** | `memgate.py`: a job starts only if its memory quote leaves 4 GB of free commit. |

## Which tool when

```
one function, starting out ──► .claude/skills/match-function/SKILL.md
                                (drive.py next → write C → drive.py try, repeat)
        │
        ├─ near miss, stuck ──► doctor.py check   (one compile: says what kind of miss)
        │                        ├─ registers / mixed / size-small ──► regtie.py diagnose | run
        │                        ├─ structure / size ───────────────► structdiff.py [--fix]
        │                        └─ constants / schedule ───────────► levers by hand (BRIEF.md list)
        │
        ├─ stuck after all that ► whatif.py search   (which cc1 decision, flipped, closes it)
        ├─ GTE (lwc2/ctc2/cop2) ► gtemap.py         (names the gte.h macro for each GTE run)
        │
        ├─ just matched ──────► twins.py replay       (credit identical copies)
        │                       nearport.py port      (port to near-copies)
        │
many functions at once:
        ├─ no AI, overnight ──► overnight.sh   (sweep m2c drafts + permuter, memgate guarded)
        ├─ many near misses ──► permute.py run (one heavy job at a time)
        └─ agents in parallel ► tools/loop/wave/   (one family per agent, then promote)

watch / steer:           browser-tools/match-viewer (live view, leave hints for a function)
what's left, by family:  nearport.py scan
struct layouts:          tools/match/structsurvey.py merge ADDR   (every local view of one global)
how far along:           progress.py   (matched, and clean = matched minus provisional)
asm-steered matches:     provisional.py list   (rework list: wave.py build writes OUT/rework.tsv)
check everything:        drive.py verify -j 4   (must end N/N matched)
```

Every tool takes `--help`; the reference for each is its section in `tools/loop/README.md`.

## Rules that are not optional

- **Only the oracle's exit 0 counts.** No "looks equivalent", no masked words.
- **Never edit what you did not write in `re/match`.** Agents work in scratch folders
  outside the repo, and a coordinator promotes their matches.
- **CPU and memory: one heavy job at a time.** Parallel permuter runs once crashed the
  machine. The permuter goes through `memgate.py` (4 GB headroom). Light jobs use
  `-j 4` at most. Agents run one compile at a time.
- **Usage: end each 5-hour window at about 80%, never at the cap.** The plan is shared,
  and hitting the cap stops other people's work. One matching agent burns about 0.16%
  of the window per minute. `tools/loop/wave/README.md` has the sizing rule.
- **One family, one agent.** Dedupe before dispatching; let tools port to members.
- **Verify before a PR, and check the PR head before calling it ready.** Run
  `drive.py verify -j 4`, push, then compare `gh pr view N --json headRefOid` with your
  pushed commit. Twice a PR was merged from a stale head and lost its last commits.
- **asm-steered C is provisional.** Register pins and empty asm statements are a last resort.
  `provisional.py scan` lists such matches (and their replayed twins) in
  `re/match/provisional.tsv`. `wave.py promote` runs it, and `progress.py` reports the clean total.
- **Keep PRs free of local paths and machine details.**

## Further reading

- `docs/MATCHING_LONG_TAIL.md`: why the last functions are the hardest, and advice for
  the next project (with postscripts on the permuter, the compiler-reading tools, and
  deduplicating work into families).
- `tools/match/README.md`: the oracle, the toolchain it reproduces, and its output format.
- `include/ac1/README.md`: the shared headers and naming.
- `docs/TOOLKIT_PROFILES.md`: the design for reusing this toolkit on another game.
