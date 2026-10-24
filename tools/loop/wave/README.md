# tools/loop/wave: running a wave of matching agents

A **wave** is a batch of matching agents (Claude Code subagents or sessions) that work
the open functions in parallel. Each agent gets its own list, its own scratch folder and
a shared brief. A coordinator (a person, or the Claude session that launched the wave)
promotes their matches into `re/match`. Waves 3-5 took this project from 443 open bodies to
85 open functions. See `docs/MATCHING_LONG_TAIL.md` for what they taught.

Start with `docs/MATCHING_GUIDE.md` if the terms below are new.

## Steps

All commands run from the repo root. `OUT` is a work folder **outside** the repo.

```bash
# 1. Port what tools can port (no agent time): near-copies of matched functions.
python tools/loop/nearport.py port OUT/ported
python tools/loop/wave/wave.py promote OUT/ported nearport        # if any matched
python tools/loop/twins.py replay -j 4                            # credit twin copies

# 2. Build the lists: one row per family, led by the member with the best draft.
python tools/loop/wave/wave.py build OUT --agents 3 --drafts PREVIOUS_WAVE_FOLDER
python tools/loop/wave/wave.py brief OUT

# 3. Launch one agent per OUT/agentN.tsv (prompt below).

# 4. As each agent reports: promote, replay twins and near-copies, verify.
python tools/loop/wave/wave.py status OUT
python tools/loop/wave/wave.py promote OUT/agent1 wave6-a1
python tools/loop/twins.py replay -j 4
python tools/loop/nearport.py port OUT/ported2                    # family members
python tools/loop/drive.py verify -j 4                            # must end N/N matched

# 5. Push a branch and open a PR. Before saying it is ready, confirm that the PR head
#    equals the pushed commit: gh pr view N --json headRefOid
```

`--drafts` can be given several times, for any folders holding earlier attempts
(`.../<unit>/func_<ADDR>.c` and their `PROGRESS.md` notes). Each draft is scored with
one compile, and the closest one becomes the row's `draft`.

## Agent prompt

> You are wave matching agent aN. Read OUT/BRIEF.md first and follow it exactly. Your
> list: OUT/agentN.tsv. Your SCRATCH: OUT/agentN (write only there; continue from
> PROGRESS.md if it exists). One compile at a time. Stop at HH:MM local time (check
> `date`) and send the report.

## Limits (read before launching)

- **CPU and memory.** Every agent runs one compile at a time, and nobody runs the
  permuter during a wave. A dozen parallel permuter jobs once exhausted this PC's
  memory and crashed it. `memgate.py` guards permuter runs. Light jobs (replay, verify)
  use `-j 4`.
- **Usage.** Agents spend a usage plan that other people share. Size the wave so each
  5-hour window ends at about **80%**, never at the cap:
  - one matching agent burns about 0.16% of the window per minute;
  - agents allowed = (80 - % used) / minutes to reset / 0.16, minus one for the
    coordinator;
  - re-check at every report, and stop agents if the window runs ahead. A stopped agent
    resumes from its PROGRESS.md.
- **One family, one agent.** The list is already one row per family. Never give the
  same family to two agents, and never let an agent port a match to the members:
  `twins.py replay` and `nearport.py port` do that for free.
- **Giants** (over 4 KB, `held.tsv`) cost the most and match least. Give them to an
  agent only when nothing else is left.

## Files

| File | What |
|---|---|
| `wave.py` | build / brief / status / items / promote |
| `BRIEF.md` | the agent brief template (rules, workflow, levers); `wave.py brief` fills in paths |
| `OUT/all.tsv` | every family, best credit per effort first |
| `OUT/agentN.tsv` | one agent's list (round-robin, so every agent starts with easy rows) |
| `OUT/ports.tsv` | families with a matched near-copy (start from that C) |
| `OUT/held.tsv` | families over `--giant` bytes |
| `OUT/drafts/` | the best draft per family |
| `OUT/agentN/` | an agent's scratch: C files, `PROGRESS.md`, `names.tsv` |
