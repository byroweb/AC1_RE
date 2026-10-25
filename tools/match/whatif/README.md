# tools/match/whatif: a cc1 that can flip one decision

A near miss usually comes down to one decision inside cc1. Examples: which pseudo gets a
register first, whether two quantities are tied, which insn the scheduler issues first,
whether cse folds a register back into its constant, whether loop hoists an insn, and
whether two tails are cross-jumped. From the C side, an agent can only nudge those
decisions blindly.

`cc1-whatif.exe` is the pinned GCC 2.7.2 cc1 plus `gcc-whatif.patch`: knobs, read from
environment variables, that flip exactly one such decision. With the variables unset it
writes the same assembly as the pinned cc1 (`python tools/loop/whatif.py selftest`
compares 300 matched C files). **It is a diagnostic only: nothing compiled with a knob
counts as a match.** The oracle (`match_fn.py`, `drive.py try`) always uses the pinned
`vendor/cc1.exe`.

| Variable | Where | Effect |
|---|---|---|
| `WHATIF_GPRIO=R:pct,...` | global.c `allocno_compare` | global-alloc priority of pseudo R scaled to pct % |
| `WHATIF_LPRIO=R:pct,...` | local-alloc.c `qty_compare` | local-alloc priority of the quantity headed by pseudo R |
| `WHATIF_NOTIE=R,...` | local-alloc.c `combine_regs` | pseudo R is never tied to another register |
| `WHATIF_NOFOLD=R,...` | cse.c `equiv_constant` | cse never replaces pseudo R by its constant |
| `WHATIF_SCHED1=U:d,...` | sched.c `priority` | sched1 priority of insn U plus d (`WHATIF_SCHED2` for sched2) |
| `WHATIF_NOMOVE=U,...` | loop.c `scan_loop` | loop never hoists insn U |
| `WHATIF_NOXJUMP=1` | jump.c `jump_optimize` | no cross-jumping |
| `WHATIF_MEMDEP=1` | sched.c `true_dependence` / `anti_dependence` | every pair of memory references is dependent |

R is a pseudo register number, as in the `-dl` dump (`Register R used ...`). U is an insn
uid, as in `-dp` assembly comments and the `-dS` / `-dR` / `-dL` dumps.

## Use

```bash
python tools/loop/whatif.py search ovl201 0x800B3748 path/to/best.c          # which knobs help
WHATIF_NOTIE=87 python tools/loop/whatif.py search ...                       # pin one, search the rest
```

`search` compiles the C once with dumps and tries every knob:
- on every pseudo;
- on the insns within `--window` of each differing word;
- on loop movables;
- the two global switches.

It reports the knobs that lower the words-off, and separately the ones that fix the
instruction order or shape with registers masked out. Then it pairs order knobs with
allocation knobs, since many misses are both at once, and stacks the helpful knobs
greedily. A result like "0 words with: local-alloc does not tie pseudo 87; sched2
priority of insn 142 +15" names exactly the decisions the C has to provoke.

## Build

```bash
tools/match/setup.sh --force --keep-build      # keeps the configured cc1 build dir
tools/match/whatif/build.sh BUILD_DIR          # the ac1_cc1_build.* dir it printed
python tools/loop/whatif.py selftest           # must report 0 differ
```
