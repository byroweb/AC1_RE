# Decomp toolkit: a generic core plus per-target profiles (design)

Status: **design only** (2026-10-24). Nothing has been refactored.

## Goal

Make the matching-decomp automation built for Armored Core reusable on other
software of similar (1990s) complexity, **without rebuilding what the
community already maintains**. The test of the design is a second target that
runs the same loop with a new profile and no edits to the core.

## Recommendation in one picture

```
            agents (Claude Code, golem, people)          disassemblers (Ghidra / IDA / Binja)
                         │  MCP                                     │  their own MCP servers
                         ▼                                          ▼
 ┌───────────────────────────────────────────────┐   investigation: types, xrefs, names
 │ CORE  (ours, target-agnostic)                 │   (+ an optional thin plugin that
 │  decomp MCP server + CLI                      │    shows match status and syncs names)
 │  loop · rank/router · ledger/progress · twins │
 │  levers · regtie · doctor/structdiff · golem  │
 │  memgate · provenance audit · guardrails      │
 └───────────────┬───────────────────────────────┘
                 │ reads one PROFILE per target
 ┌───────────────▼───────────────────────────────┐
 │ PROFILE  (per target, data + small adapters)  │
 │  units/segments · ISA · compiler recipe ·     │
 │  SDK signatures · headers · emulator hook     │
 └───────────────┬───────────────────────────────┘
                 │ drives
 ┌───────────────▼───────────────────────────────┐
 │ SUBSTRATE  (community tools, used as-is)      │
 │  splat / decomp-toolkit · objdiff · m2c ·     │
 │  decomp-permuter · coddog · decomp.me         │
 │  compilers · reccmp (MSVC)                    │
 └───────────────────────────────────────────────┘
```

- **The host is an MCP server plus a CLI, not a plugin inside a disassembler.**
  The loop runs queued, memory-gated compile-and-compare jobs for hours.
  Ghidra, IDA and Binary Ninja are built around an interactive analysis
  database. They already have MCP servers, so agents use them alongside ours.
- **The oracle stays a plain command with an exit code.** MCP wraps it and
  never replaces it. A match is only ever "the oracle printed MATCH".

## Prior art: what to adopt and what is new

| need | existing | decision |
|---|---|---|
| split a binary into asm/data/C units | **splat** (MIPS: N64, PS1, PS2), **decomp-toolkit** (GameCube/Wii) | adopt; the profile's unit layout becomes their config |
| compare our object with retail | **objdiff** (ARM, ARM64, MIPS, PowerPC, SuperH and x86 backends) | adopt as the default diff backend. Keep `match_fn.py` as the AC1 backend until objdiff agrees with it on every manifest row |
| MSVC / x86 PE comparison | **reccmp** (LEGO Island project; annotation-driven, PDB-based) | adopt for MSVC targets |
| old compilers | **decomp.me** (open source, Django + Next.js, Docker local setup, docs for adding compilers) | use its compiler packaging where a target's compiler is closed. Keep self-built compilers where we must patch or instrument them (regtie) |
| first C draft | **m2c** (MIPS, PowerPC) | already used; Ghidra or IDA decompiler output for x86/ARM |
| random search on near misses | **decomp-permuter** | already used. Lesson from Chris Lewis's N64 run: permuter output can overfit and mislead agents, so keep it behind the doctor's classification |
| "which matched function looks like this one" | **coddog** (bounded Levenshtein distance over opcode sequences) | adopt next to `twins.py`. Twins finds exact clones; coddog finds near relatives to use as reference C, which Lewis reports was his most effective change in the long tail |
| disassembler access for agents | **ida-mcp** (Hex-Rays), **ida-pro-mcp**, GhidraMCP, REA | use as-is |
| independent byte verification | **decomp-compat-kit** | worth reviewing for the provenance plan ([FULL_LINK_PLAN.md](FULL_LINK_PLAN.md)) |

**New here, and what the core packages:**
- the oracle-first loop with ledger and progress;
- the tier router;
- twin crediting;
- semantics-neutral levers;
- **regtie**: compiler-guided search on register ties, read from cc1's own allocation dumps;
- the doctor + structdiff classification;
- the local-LLM golem tier;
- the memory gate;
- the provenance audit.

I found no packaged, target-agnostic version of this layer. The public agent
decomps so far are per-project scripts, for example Lewis's `run.py`/Nigel
orchestrator and an N64 project combining splat, m2c, asm-differ, permuter
and IDO.

## Current tools sorted into the three layers

A rough grep of `tools/loop` and `tools/match` for AC1/PS1-specific strings
puts the target knowledge in about a fifth of the ~12,900 lines, almost all
of it in four files.

| tool | layer | what is target-specific today | after the split |
|---|---|---|---|
| `match_fn.py` | core (oracle) + profile | cc1 flags, maspsx, `.align 3 → 2` rodata fix, `--ovl` unit lookup | oracle driver in the core; the compile recipe, rodata fix and unit lookup come from the profile; compare via objdiff, or the built-in comparer kept as a backend |
| `inventory.py` | profile | PS-X EXE carving, FDAT entries, mission tables, PsyQ ranges | becomes profile data (splat config + `CODE_UNITS`-style table) plus a small loader adapter |
| `units.py` | profile | unit names, load bases, aliases | profile data |
| `ctx.py` | profile | AC1 header context for m2c | profile headers |
| `drive.py` | core | paths under `re/match/` | core; source root from the profile |
| `rank.py`, `router.py`, `ledger.py`, `progress.py`, `make_queues.py`, `merge_manifest.py` | core | feature names assume MIPS (GTE ops, jump tables) | core, with ISA-specific features supplied by the profile |
| `twins.py` | core | masking rules for MIPS relocations | core; masking per ISA via objdiff's relocation info |
| `sweep.py`, `m2cfix.py` | core + ISA | m2c fix-ups are MIPS/GCC idioms | core; fix-up packs per (ISA, compiler) |
| `levers.py`, `permute.py`, `variants.py` | core + compiler | levers encode GCC 2.7.2 habits | core; lever packs per compiler family |
| `regtie.py` | core, **open-compiler only** | reads GCC 2.x `-dl`/`-dg` dumps and COFF debug info | core module enabled by a profile capability (`instrumentable: gcc2`) |
| `doctor.py`, `structdiff.py` | core | the switch-table rule is cc1's | core; compiler heuristics from the lever pack |
| `golem.py` | core | prompt templates mention AC1 | core; prompt context from the profile |
| `memgate.py`, `heartbeat.py`, `overnight.sh` | core | none, apart from paths | core |
| `tools/link/p0_readiness.py` and the provenance audit | core + profile | unit ranges, mission-host map, exe bss bounds | core audit; section map and link graph from the profile |
| `browser-tools/match-viewer` | core UI | none known | core UI, reads the core ledger |

## What a profile contains

One directory per target, mostly data. A sketch for AC1:

```toml
# profiles/ac1/profile.toml
name        = "Armored Core (SLUS-01323)"
isa         = "mips-r3000-le"
source_root = "src"                     # matched C (the FULL_LINK_PLAN Q3 move)

[units]
layout   = "splat"                      # or "decomp-toolkit", "pe"
config   = "profiles/ac1/splat/*.yaml"
table    = "overlays/CODE_UNITS.tsv"    # hashes, bases, aliases
link     = "profiles/ac1/link_graph.toml"   # exe → 201..204 → missions, the exe↔ovl201 cycle

[compiler]
family   = "gcc-2.7.2-psx"
recipe   = "profiles/ac1/toolchain/setup.sh"   # builds + patches cc1; records hashes
cpp_args = "-undef -D_LANGUAGE_C -D__psx__ ..."
cc1_args = "-O2 -mips1 -mcpu=3000 -G0 -fpeephole ..."
asm_shim = "maspsx --aspsx-version=2.56"
as_args  = "-EL -march=r3000 -no-pad-sections -G0"
fixups   = ["rodata-align-3-to-2"]
capabilities = ["instrumentable:gcc2", "coff-debug"]   # turns regtie on

[diff]
backend  = "objdiff"                    # or "match_fn" (legacy), "reccmp"

[symbols]
sdk      = "psyq-3.7"                   # signature set for library labels
markup   = ["ac1_markup.tsv", "include/ac1/symbols.tsv"]
headers  = "include/ac1"

[runtime]                               # optional: behavioural checks, shift test
emulator = "duckstation-mcp"
```

## Compiler tiers decide which tools a target gets

| tier | examples | loop, router, ledger, twins, doctor | permuter, levers | regtie | byte-matching claim |
|---|---|---|---|---|---|
| **A: open and rebuildable** | GCC 2.x for PS1/PS2, agbcc (GBA), KMC GCC (N64) | yes | yes | **yes** | yes |
| **B: closed binary** | IDO (N64), MSVC 4–6 (Win9x), Metrowerks (GameCube/DS/Mac), Watcom (DOS) | yes | yes | no, unless its allocator is reimplemented | yes |
| **C: compiler unknown or missing** | anything without the toolchain | yes | partly | no | no: a functional decomp with an emulator differential check |

## MCP surface of the core

The tools are thin wrappers over the CLI, so the CLI stays the source of
truth:

| tool | does | returns |
|---|---|---|
| `status` | progress per unit, open queue sizes | table |
| `next_function` | next work package (router-ranked; `--similar` adds coddog neighbours) | asm, m2c draft, similar matched C, oracle command |
| `try_c` | compile a C file with the profile recipe and compare | MATCH / diff / doctor class |
| `doctor` | classify a near miss and name the tool for it | class + plan |
| `regtie`, `permute`, `levers` | run that tier on a body, inside the memory gate | best C, score |
| `credit_twins` | replay a matched file onto its clones | rows added |
| `verify` | re-run every manifest row | pass/fail list |
| `provenance` | byte classes per unit | table, treemap JSON |

**Guardrails live in the core**, which Lewis's write-up shows is necessary
(one of his unattended agents edited the expected SHA-1 to force a "match"):
- only the oracle writes manifest rows;
- hashes, generated files and the manifest are write-protected for agents;
- every MATCH is re-verified with the plain profile compiler. Regtie and the
  permuter may steer with instrumentation, but a match must compile from
  ordinary C.

Design note: the 2026 Echo paper found one agent working through many
functions in a single session did worse than independent per-function
queries, because each function got less reasoning. The MCP surface is
per-function on purpose, so it suits short independent sessions as well as a
lane running for hours.

## Migration plan for AC1 (no big-bang)

| step | change | exit test |
|---|---|---|
| M0 | this design | agreed |
| M1 | move AC1 constants out of `units.py`, `inventory.py`, `ctx.py` and `match_fn.py` into `profiles/ac1/` | `drive.py verify` unchanged; P0 audit unchanged |
| M2 | objdiff as an optional diff backend | objdiff and `match_fn.py` agree on all manifest rows |
| M3 | coddog neighbours in `next_function` | measurable effect on open-body hit rate |
| M4 | package the core (Python package + MCP server); AC1 becomes "a repo with a profile" | an agent runs the full loop through MCP only |
| M5 | **second profile pilot.** Easiest: another PsyQ PS1 title (tier A, same compiler family). Then a tier-B target through decomp.me's compiler packaging | the loop runs with no core edits |

The full-link work ([FULL_LINK_PLAN.md](FULL_LINK_PLAN.md)) fits this split
directly: the linker scripts and section map become profile data, and the
provenance audit is core.

## Risks and open questions

- **Compare semantics.** `match_fn.py` resolves or masks relocations in its own
  way. objdiff must reproduce its verdict exactly before it can replace it,
  hence the M2 exit test.
- **Windows.** decomp.me's compiler packaging targets Linux (Wine for MSVC and
  some closed compilers), so on Windows tier-B profiles probably need WSL or
  Docker.
- **Compiler licences.** Closed compilers can't be redistributed. Profiles
  reference them and never ship them.
- **Community norms.** At least one AI-decomp project (decomp.wtf) was archived
  after pushback from the decompilation community. Publishing a reusable
  agent toolkit calls for clear attribution of the community tools it builds
  on and honest labelling of machine-made matches.
- **Overfitting.** Permuter- or regtie-found C can be unidiomatic. The doctor
  should prefer readable rewrites, and matched C is reviewed before it becomes
  reference material for coddog neighbours.

## Sources

- objdiff architecture support: [objdiff-core on docs.rs](https://docs.rs/crate/objdiff-core/3.8.1), [objdiff repository](https://git.axiodl.com/encounter/objdiff/src/commit/03f2bcb8b133b50c42a07c48857f861a1a16d5ec/objdiff-core)
- reccmp: [PyPI](https://pypi.org/project/reccmp/0.1.4/), [LEGO Island decomp](https://git.mattkc.com/isledecomp/isle)
- decomp.me architecture and self-hosting guides: [docs index](https://docsearch.algolia.com/mcp/docs/repo/decompme/decomp.me)
- coddog, the similarity finder, permuter overfitting, the SHA-1 guardrail incident: [The Long Tail of LLM-Assisted Decompilation](https://blog.chrislewis.au/the-long-tail-of-llm-assisted-decompilation/), [Using Coding Agents to Decompile N64 Games](https://blog.chrislewis.au/using-coding-agents-to-decompile-nintendo-64-games/)
- MCP servers and verification kits: [awesome-rea](https://github.com/yibie/awesome-rea) (ida-mcp, ida-pro-mcp, REA, decomp-compat-kit, decomp.wtf)
- per-function vs agentic batching: [Echo (arXiv 2609.18706)](https://arxiv.org/pdf/2609.18706)
- survey: [AI-assisted decompilation for retro games](https://www.retroreversing.com/ai-assisted-decompilation)
