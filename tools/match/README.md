# tools/match — byte-match oracle for one function

`match_fn.py` compiles one hand-written C function with the retail Armored Core
toolchain and compares the bytes with the retail binary.  It is deliberately a
dumb, standalone tool with an exit code: a Claude session (or a human) can argue
with a decompiler, not with this.

```
tools/match/setup.sh                       # once per machine (see "Install")
python tools/match/match_fn.py 0x8004F7E8 tools/match/tests/func_8004F7E8.c
...
MATCH func_8004F7E8 40                     # exit 0
```

Only exit status 0 with a final `MATCH` line counts as matched.

## What it reproduces

Retail SLUS-01323 (code-identical to SCUS-94182, verified 2026-10-18) was built with
PsyQ 4.0 `CC1PSX` = GCC 2.7.2 "SN32 3.7.0002", assembled by ASPSX 2.56 and linked
4-byte packed.  `match_fn.py` runs the same stages:

| stage | tool | flags |
|---|---|---|
| preprocess | host `cpp` (MinGW/GCC) | `-E -P -undef -nostdinc -D_LANGUAGE_C -D_MIPSEL -DMIPSEL -Dmips -D__mips__ -D__GNUC__=2 -D__GNUC_MINOR__=7 -D__psx__ -I<srcdir> -I tools/match/include` |
| compile | GCC 2.7.2 psx `cc1` | `-O2 -mips1 -mcpu=3000 -w -funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return -fcommon -msoft-float -mgas -fgnu-linker -gcoff -G0 -quiet` |
| assemble | [maspsx](https://github.com/mkst/maspsx) `--aspsx-version=2.56` then mipsel GNU `as` | `-EL -march=r3000 -mtune=r3000 -no-pad-sections -G0` |
| extract | own ELF reader | bytes of the named `FUNC` symbol in `.text`, relocations applied or masked |
| compare | word by word | against `<vaddr>` in the overlay / exe |

The `cc1` output of decompals' gcc-2.7.2-psx is identical to `CC1PSX` on every
function tried so far (the pin comes from the Wyrelade compiler probe over nine
candidate builds: only this one reproduces the unfilled epilogues, the `switch`
tables and the divide-by-constant sequences).

### Known gotcha handled: jump tables `.align 3`

cc1 writes `.align 3` before every `switch` jump table in `.rdata`; the PsyQ
linker packed objects 4-aligned, so retail tables sit at 4-byte boundaries.
`match_fn.py` rewrites `.align 3` → `.align 2` inside `.rdata`/`.rodata` before
assembling (`--no-rodata-align-fix` keeps cc1's output).  The text bytes do not
depend on it, but `--rodata VADDR` does: with it, the `lui/addiu` pair that loads
the table address is checked against the retail table address.

### Relocations / external symbols

A function that calls `jal other` or reads a global emits `R_MIPS_26` /
`R_MIPS_HI16+LO16` relocations.  For each one `match_fn.py`:

1. resolves it if the address is known: `--defsym NAME=0xADDR` (repeatable),
   any `--syms FILE.tsv` (`addr<TAB>name` or the `ac1_markup.tsv` layout, which is
   read by default; `--no-markup` disables that), the function's own `.text`
   (recursion, local labels), `--rodata` / `--data` for the object's own sections;
2. otherwise **masks** the immediate field of that word (26 bits or 16 bits) and
   prints `WARNING: n word(s) masked, unresolved symbol(s): ...`.  The rest of the
   word (opcode, registers) is still compared.  A masked MATCH is a match of the
   code shape, not of the addresses; the JSON carries `masked_words` and
   `unresolved_symbols` so a loop can refuse to accept it.

Resolution keeps `%hi` carry semantics (`hi = (S + AHL + 0x8000) >> 16`).

## Usage

```
match_fn.py <vaddr> <src.c> [options]

  --bin PATH --base VADDR   compare against this file (VADDR = address of byte 0)
  --ovl N                   overlay number for addresses in overlay space (default 201;
                            201..204 all load at 0x8004ADA0 so the address alone is ambiguous)
  --fn NAME                 function symbol to extract (default: the only function in the file)
  --defsym NAME=ADDR        address for an external symbol (repeatable)
  --syms FILE.tsv           extra symbol table(s); --no-markup ignores ac1_markup.tsv
  --rodata VADDR, --data VADDR   retail address of this object's .rodata / .data
  --expect-size N           retail size of the function; a different size is a MISMATCH
  --json [PATH]             write the facts as JSON (stdout when PATH is omitted or "-")
  --keep DIR                keep fn.i, fn.s (cc1), fn.fixed.s, fn.maspsx.s, fn.o, retail.bin, ours.bin
  -I DIR, -D MACRO          extra cpp include dirs / macros
  --context N               show only N words around each difference (default: whole function)
  -q                        print only the final line (plus JSON)
  --cc1 / --as / --maspsx / --cpp / --aspsx-version   override the toolchain
```

Binary selection without `--bin`: addresses in `0x80011000..0x8004ADA0` use the main
exe, carved from the disc image in `$AC1_BIN` (raw sectors 39–121, cached in
`vendor/cache/`); addresses from `0x8004ADA0` use `disc_map/overlays/ovl<N>_*.bin`
via `overlays.json` (looked up in this checkout, the main checkout of a worktree, or
`$AC1_OVERLAYS`).

### Output

Last stdout line, always exactly one of:

| line | exit | meaning |
|---|---|---|
| `MATCH <fn> <n bytes>` | 0 | all `n` bytes of `<fn>` equal the retail bytes at `<vaddr>` (masked words: see warning) |
| `MISMATCH <fn> <offset>` | 1 | first differing word at `<offset>` (hex) from the function start, or the size differs from `--expect-size` |
| `ERROR: <reason>` | 2 | toolchain missing, cpp/cc1/maspsx/as failed, bad address, file not found |
| (argparse usage) | 3 | bad command line |

On MISMATCH a side-by-side disassembly is printed (`!` marks differing words, `m`
masked ones), e.g.

```
  addr      retail                             | ours
  8004f7e8  lhu v0,64(a0)                      | lhu v0,64(a0)
! 8004f7ec  lhu v1,68(a0)                      | lhu v1,66(a0)
  ...
6 differing word(s)
MISMATCH func_8004F7E8 0x4
```

The temp work dir (`fn.i`, `fn.s`, `fn.maspsx.s`, `fn.o`) is always deleted.  Use
`--keep DIR` to keep it in DIR, or `--keep-on-fail` (or env `AC1_MATCH_KEEP_ON_FAIL=1`)
to keep the temp dir only on MISMATCH/ERROR; its path is printed on stderr.

A MATCH says the first `n` bytes at `<vaddr>` equal your function.  It does not
by itself prove the retail function is not longer; pass `--expect-size` with the
size from the disassembly (next function start − `<vaddr>`) when that matters.

### JSON facts (`--json`)

`result`, `fn`, `vaddr`, `src`, `src_sha1`, `bytes`, `first_diff_offset`,
`diff_words`, `masked_words`, `unresolved_symbols`, `resolved_symbols`, `bin`,
`bin_label`, `bin_offset`, `cc1`, `cc1_flags`, `aspsx_version`, `as_flags`, `error`,
`aligned_words` (retail words, `--expect-size` long, that difflib pairs in order with an
equal word of ours: a closeness that survives an inserted instruction; masked words are
compared unmasked; all words on a MATCH).

## Install

```
tools/match/setup.sh            # Git Bash on Windows, or any Linux/macOS shell
```

Everything lands in `tools/match/vendor/` (gitignored, never committed) and is
recorded in `vendor/VERSIONS.txt`; `vendor/toolchain.json` tells `match_fn.py`
where things are.  Pins (all in `setup.sh`):

| component | pin |
|---|---|
| maspsx | `mkst/maspsx` commit `670417cf5bd5d9dae4a962b18f400363670da5c5` (2026-10-17, = upstream HEAD on 2026-10-18), cloned with `core.autocrlf=false`, plus `patches/maspsx-*.patch` |
| cc1, Linux | `decompals/old-gcc` release **0.17** `gcc-2.7.2-psx.tar.gz`, sha256 `500a459b3485e885a8d302cac23c2a4632f3900e03a09153f6190699fd723571` |
| cc1, macOS | same release, `gcc-2.7.2-psx-macos.tar.gz` |
| cc1, Windows | built locally: GNU `gcc-2.7.2.tar.gz` (sha256 `7cd8bce5c3aeec59a72ecc2d3d5123864a817b14cdbd0680b1a969c3bccc5da5`) + the `patches/` of `decompals/old-gcc` commit `cf70ec58bce58128732062bc9930045c82904cbb`, following `gcc-2.7.2-psx.Dockerfile` |
| as / objdump | whatever mipsel GNU binutils is on PATH (`mipsel-none-elf-` PCSX-Redux toolchain on this PC, binutils 2.45; `mipsel-linux-gnu-` on Debian) |
| cpp | host `cpp` on PATH (MinGW-w64 16 here) |

### Windows notes

decompals publishes no Windows binary and WSL/Docker may be absent, so on
Windows `setup.sh` builds `cc1.exe` itself with the MinGW-w64 `gcc` on PATH
(WinLibs via winget here).  It applies exactly the old-gcc recipe (varargs sed,
`obstack-2.7.2.h`, `configure`, `config.sub`, `mipsel-2.7`, `psx-2.5.7` patches;
`--target=mips-sony-psx --with-endian-little --with-gnu-as`) plus the bits a
MinGW host lacks: stub `<sys/times.h>`, `<sys/resource.h>`, `<sys/param.h>`,
`<sys/file.h>`, `<sys/wait.h>` and a `compat.o` with `index`, `rindex`, `bcmp`,
`bcopy`, `bzero`, `sbrk`.  The build is checked with old-gcc's own post-build
tests (little-endian `lbu`, 32-bit `HOST_WIDE_INT`, `-msoft-float` present).  It
takes a few minutes; needs `gcc`, `make` (or `mingw32-make`), `patch`, `curl`, `tar`.

Caveats of the local build: it is a 64-bit host build (old-gcc uses `-m32`); on
Windows `long` is still 32 bits so `HOST_WIDE_INT` is right, but pointers are
64-bit.  That broke floating-point constants until `patches/gcc-const-double-slots.patch`
(see "Our gcc patches" below).  If a function refuses to match for no visible
reason, cross-check with the official tarball (`setup.sh --wsl` once WSL is
installed, or on a Linux box).
Also: this cc1 must always get `-G0` (the retail flag line has it); with a
non-zero `-G` the compiler tries to open a Unix-style temp file and fails.

Alternatives: `setup.sh --wsl` downloads the Linux tarball and writes
`vendor/cc1.sh`, which runs it through `wsl.exe` (needs `wsl --install` first;
untested here since this PC has no WSL).  `setup.sh --cc1 PATH` registers a cc1
you already own, e.g. PsyQ 4.0 `CC1PSX.EXE` (a native Win32 binary, the actual
retail compiler) — nothing is downloaded for it.

### Our maspsx patches

`setup.sh` applies every `tools/match/patches/maspsx-*.patch` to the pinned clone
(idempotent; recorded with its sha256 in `VERSIONS.txt`).  Currently one:

- `maspsx-lo-load-nop.patch`: a load whose offset is already `%lo(sym)` off a base
  register (`lw/lh/lhu/lb/lbu/lwl/lwr $r,%lo(sym)($base)`) is a single instruction
  that ASPSX does not route through `$at`, the same as the store form maspsx already
  handles.  Unpatched maspsx treats it like the bare-symbol macro `lw $r,sym($base)`
  and drops the load-delay `nop` when the previous instruction loads `$base`.  The
  fact comes from the Wyrelade AC1 decomp (checked by him against real ASPSX); the
  patch text is ours.  `tests/test_maspsx_patch.py` fails without it.  The 53
  manifest functions are unaffected (`check_all.sh` output identical before/after).

A checkout whose `vendor/maspsx` predates this just needs a plain
`tools/match/setup.sh` (no `--force`): it patches the existing clone in place, CRLF or
not, and re-clones only maspsx if the clone is at the wrong commit or hand-edited.
cc1 is never rebuilt for this.

### Our gcc patches (Windows build only)

The Windows `cc1.exe` build applies every `tools/match/patches/gcc-*.patch` after the
old-gcc recipe and links with a 64 MB stack reserve.  `setup.sh` records both (patch
names + sha256, the reserve) in `vendor/cc1.exe.build`; when that does not match the
tree, a plain `tools/match/setup.sh` rebuilds `cc1.exe` (otherwise only `--force` does).

- Stack reserve (`-Wl,--stack,67108864`): gcc 2.7.2 recurses over a function's insn
  chain, so its stack use grows with the length of a basic block.  With MinGW's default
  2 MB reserve, cc1.exe died with no message (a stack overflow) on long
  straight-line code.  A 4000-call function reproduces it every time.  The shop
  constructor ovl201 0x8008C3EC (108 `MoveImage` calls in a row) was reported crashing
  in a matching run.  Its final C compiles with either reserve, so that report is not
  reproduced; the 4000-call case is.
  `tests/test_cc1_stack.py` and the build self-test compile the 4000-call function.
- No ASLR (`-Wl,--disable-high-entropy-va -Wl,--disable-dynamicbase`): gcc 2.7.2 calls
  some pointer-returning functions with no declaration in scope (K&R implicit `int`).
  On the 64-bit host that truncates the pointer to 32 bits.  A scan of cc1's sources
  with `-Wimplicit-function-declaration` finds `negate_rtx` (reorg.c), `alter_subreg`,
  `emit`, `emit_line_note`, `expand_expr`, `label_rtx`, `remove_death`,
  `replace_regs`, `reorder_blocks`, `bc_xstrdup`, plus `malloc`/`realloc` (stmt.c,
  toplev.c) and `index` (c-common.c).  With high-entropy ASLR the heap sits above
  4 GB and the truncated pointer crashes; without it, image and heap stay below 4 GB
  and the truncation is harmless.  The build self-test checks the PE header bits.

Patches, currently two:

- `gcc-reorg-negate-rtx.patch`: declares `negate_rtx` in reorg.c.  It is the one
  implicit call seen crashing: `fill_slots_from_thread` (delay-slot filling) segfaulted
  on loops whose delay slot gets an increment plus its inverse fix-up.  The wave-3
  matching run hit it on ovl202 0x8004C854, 0x80052350 and 0x80067AB4 (the first two
  then matched with an ASLR-off cc1 copy, and match with the patched build).

- `gcc-const-double-slots.patch`: a `CONST_DOUBLE` keeps its value in rtx slots that
  are each an `rtunion`, i.e. pointer-sized.  gcc 2.7.2 moves a real value in and out
  with `bcopy` to `&CONST_DOUBLE_LOW (r)`, which assumes a slot is one `HOST_WIDE_INT`
  (true for old-gcc's `-m32` build).  On the 64-bit MinGW host a slot is 8 bytes and
  `HOST_WIDE_INT` 4, so `REAL_VALUE_FROM_CONST_DOUBLE` read the low word plus 4 bytes
  of slot padding: every float/double literal came out with a zero or garbage high
  word (`x * 1.15` gave `.word 0x66666666, 0x00000150`, `1.5f` gave `0`).
  `mips_const_double_ok` judged constants on the same garbage, so every literal went to
  the constant pool; with the patch a literal in 1e-300..1e300 (float 1e-38..1e38) is
  an `li.d`/`li.s` immediate, which maspsx expands to `lui`/`ori` as retail has it, and
  only the rest is pooled.  The patch copies one `HOST_WIDE_INT` per slot (`real.h`,
  `emit-rtl.c`, `varasm.c`, `rtl.c`).  `tests/test_cc1_float_const.py` fails without
  it; the build's own self-test checks `1.15` and `1e305` too.

Environment overrides read by `match_fn.py`: `AC1_CC1`, `AC1_AS`, `AC1_MASPSX`,
`AC1_CPP`, `MIPS_PREFIX`, `AC1_BIN` (disc image, for exe addresses),
`AC1_OVERLAYS` (directory with `overlays.json`).

## Tests

```
python -m pytest tools/match/tests      # or: bash tools/match/tests/run.sh
```

`func_8004F7E8.c` (overlay 201, 10-instruction leaf: three `u16` fields bumped)
must MATCH (`test_match_fn.py`); `func_8004F7E8_perturbed.c` (two statements swapped) must MISMATCH
at `0x4` with a side-by-side diff; a third case (`func_80090C38`, reads a global)
checks masking, `--defsym` resolution and a wrong `--defsym` → MISMATCH.  The tests
skip (never pass vacuously) when the toolchain or the carved overlay is missing.
`test_maspsx_patch.py` runs maspsx + `as` on tiny `.ent`-framed sequences and checks
the load-delay `nop` before `l* $r,%lo(sym)($base)` (needs only the toolchain).

`test_objdiff_prep.py` covers `objdiff_prep.py`: the object writer, path parsing, masks, the
config shape, and the worked example building the same bytes as its target.

`test_variants.py` covers `variants.py` (below): edit parsing, and a batch on the perturbed
source whose swap variant must come back as a clean MATCH.

## Variant sweeps (`variants.py`)

One parallel batch of small rewrites of a near miss, instead of one edit per oracle run:

```
python tools/match/variants.py 0x<VADDR> re/match/ovl202/func_<VADDR>.c --edits try.txt --levers swap,commute --pairs
```

`try.txt` holds `## label` blocks of `-old` / `+new` lines (several replacements per
variant allowed; the old text must occur once, whitespace-tolerant); `--replace OLD NEW`
adds one more from the command line and `--levers` adds `tools/loop/levers.py` rewrites.
Every variant is compiled with the oracle's pipeline in process (`tools/loop/permute.py`;
about 60 compiles/s at `-j 24` on a 368-byte body) and ranked by differing words (+2 per
word of size difference) and by retail words difflib cannot pair.  `--pairs` then tries
every two edit variants that each improved on the input together.  Score-0 variants are
re-run through `match_fn.py` and only a clean MATCH there gives exit 0.  The unit comes
from `--ovl`/`--unit`, the file's directory or the exe range; the size from
`--expect-size` or `tools/loop/functions.tsv`.  The input is untouched unless `--apply`;
`--out F` writes the best variant, `--keep DIR` every variant plus `variants.tsv`.
Run `variants.py -h` for the full format.

## objdiff workstation (`objdiff_prep.py`)

[objdiff](https://github.com/encounter/objdiff) is a free GUI that shows the target
and your current build side by side, instruction by instruction, and rebuilds when a
source file changes.  `objdiff_prep.py` makes this repo an objdiff project, so you can
edit a function's C in your own editor and watch the diff update, without an agent.

```
python tools/match/objdiff_prep.py config --unit ovl201      # objdiff.json + one target .o per open function
python tools/match/objdiff_prep.py config                    # every unit (exe, ovl201..204, fdatNNN)
python tools/match/objdiff_prep.py config --with-matched --prebuild -j 16   # + manifest rows, bases built now
python tools/match/objdiff_prep.py one ovl201 0x8004F7E8     # build both objects for one function, print SAME/DIFF
```

`config` writes `objdiff.json` at the repo root and `build/objdiff/target/<unit>/func_<ADDR>.o`
(both gitignored).  Each unit in objdiff is one function, named `<unit>/func_<ADDR>`:

- **target** = the retail bytes `match_fn.py` compares against (`--expect-size` long, from
  `tools/loop/functions.tsv`);
- **base** = `re/match/<unit>/func_<ADDR>.c` compiled by `match_fn.py`'s own functions with
  the argv `drive.py try` would pass (open functions: unit flags, `--expect-size`,
  `--rodata` from `jt_addrs`, `--syms` when the file includes `ac1/ac1.h`, auto
  `--defsym` for `func_<ADDR>`/`D_<ADDR>`) or the manifest row's argv (matched functions).
  objdiff builds it itself through `objdiff_prep.py build <path>` whenever a watched file
  (`*.c`, `*.h`, `*.inc`, `ac1_markup.tsv`, `include/ac1/symbols.tsv`) changes.
- A function with no C file yet gets `drive.py next`'s skeleton the first time objdiff
  builds it: an empty `void func_<ADDR>(void)` with the m2c draft in a comment.

Both objects are flat: `.text` holds final bytes with relocations already applied, and a
word the oracle masks (unresolved symbol) carries the retail value in its masked field.
So objdiff compares exactly what the oracle compares, and "100%" in objdiff means the
oracle will say MATCH (checked: `func_8004F7E8` 100% both ways; an open skeleton 10.8%).
The cost: `jal`/`lui` targets show as numbers, not symbol names.  Jump tables in
`.rodata` are not shown; `--rodata` still checks the `lui/addiu` that loads them.

### Setup (once)

1. Download `objdiff-windows-x86_64.exe` (GUI) from the objdiff releases page (3.8.2 is
   what this was tested with; `objdiff-cli` is optional) and put it anywhere.
2. `python tools/match/objdiff_prep.py config --unit ovl201` (rerun after matches land or
   to switch units; it is quick: ~10 s for ovl201, under a minute for all 972 open functions).
3. Start objdiff, *File > Project* (or the project button), pick the repo root.  The
   sidebar lists the units by unit; click one.  The build command and the MIPS options
   (`o32`, `r3000gte`) come from `objdiff.json`.

The build runs in the repo root with the toolchain from `tools/match/setup.sh`, so the
same `AC1_BIN` / `MIPS_PREFIX` environment the oracle needs must be visible to objdiff
(set them as user environment variables, not only in a Git Bash profile).

### Match, check, commit

1. Pick a function in objdiff and edit `re/match/<unit>/func_<ADDR>.c`; save; objdiff
   rebuilds and re-diffs.  Hints from `drive.py try` and the match-function skill's
   diff-pattern table apply unchanged.
2. When objdiff shows no differences, run the judge:
   `python tools/loop/drive.py try <unit> <vaddr>`.  Exit 0 = clean MATCH; it appends the
   manifest row and the ledger row and prints the `git commit` command (`--commit` runs it).
3. Commit the C file and `re/match/manifest.tsv` (and the ledger), then rerun
   `objdiff_prep.py config` so the function moves to the matched side.

objdiff's percentage is a guide, not a verdict: only `match_fn.py` exit 0 (through
`drive.py try`) counts.

## Coverage tagging from play (`tools/duckstation/coverage_tag.py`)

Which screen a function runs on is a strong naming hint, and you know the game.  This
script puts execute breakpoints on functions while you play and tags each one with the
activity you type.  What to play and which labels to type: `docs/PLAY_COVERAGE_SWEEP.md`.

```
python tools/duckstation/coverage_tag.py --check                  # MCP up? which units are loaded? call latency
python tools/duckstation/coverage_tag.py --context title          # every unit, every function; type labels as you play
python tools/duckstation/coverage_tag.py --set unseen --context garage           # only functions never seen yet
python tools/duckstation/coverage_tag.py --unit mission --max-armed 300 --context combat
python tools/duckstation/coverage_tag.py --context assembly --capture ac_calc_performance
```

Start DuckStation (the MCP fork, port 2346, see `tools/README.md`) with the game running,
then the script in a terminal.  Type a label and Enter when the screen changes
(`garage`, `shop buy`, `briefing`, `boost`), `?` for status, `q` to stop.

- **Which functions** (`--set`): `all` (default), `unnamed` (no `name` row in hints.tsv and
  no function entry in `include/ac1/symbols.tsv`), `unseen` (no coverage.tsv row yet), or
  `open` (not in the manifest; the old default, empty now).  PsyQ library code never.
- **Which units** (`--unit`, repeatable): `all` (default), `front` (exe + ovl201), `mission`
  (exe + ovl202 + the story mission overlays), `vs` (exe + ovl203/204 + the arena
  overlays), or unit names (`exe`, `ovl202`, `fdat060`).  Units whose image is identical to
  another (fdat168 = fdat166) are counted under the first.
- Each chosen function of a loaded unit gets an execute breakpoint.  A hit is counted
  under the current label, and the breakpoint comes off after `--hits` hits (default 1),
  so the game stutters only while new code runs.
- A new label re-arms the functions, so a function that runs on several screens gets one
  row per screen.  Re-arming skips:
  - functions that already have a row under that label, in this run or an earlier one;
  - functions seen under `--max-contexts` labels already (default 8, the number at which
    the hint says "broad").
- Overlays share load addresses: ovl201..204 at `0x8004ADA0`; story missions at
  `0x801C4B40`, mode2 at `0x801C8460`, mode1 at `0x801B80C8`.
  - A unit is armed only while a 64-byte probe window of its image is in RAM. The window
    is picked where the image differs from its slot mates.
  - The check runs every `--sync` seconds, with one read per slot when its windows fit in
    64 KB.
  - A hit only counts when the word at the PC is that unit's retail word.
- `--max-armed N` keeps at most N coverage breakpoints in place and rotates the rest in
  every `--rotate` seconds. Functions seen on the fewest screens go first.  Use it if the
  game crawls with a whole overlay armed.
- `--capture` (repeatable: `unit:0xADDR`, an exe address, or a name from hints.tsv)
  records arguments and return values:
  - on entry, a0..a3 and `--deref` bytes (default 16) behind each argument that points
    into RAM;
  - then a breakpoint on `ra`; when it fires with the entry `sp`, v0/v1 and the same bytes
    again.
  - Up to `--captures` calls (default 5) per function per label are recorded.
- A pause that is not on one of its breakpoints (yours, from the emulator menu) is left
  alone.  On exit it removes its breakpoints and resumes the game if it stopped on one.

Output, merged with what is there:
- `re/match/coverage.tsv`: `unit vaddr context hits` (hits = times counted; with
  `--hits 1`, the number of labels/re-arms the function ran under);
- `re/match/hints.tsv` (`unit vaddr kind text timestamp`, shared with the match viewer):
  one `hint` row per function hit in this run, `play coverage: garage (3), shop (1)`,
  skipped when that function's newest play-coverage row already says the same;
- `re/match/calls.tsv` (`--capture`): `unit vaddr context a0 a1 a2 a3 v0 v1 ra in out
  timestamp`, one row per recorded call (`in`/`out`: `a0:hex;a2:hex` bytes behind
  pointer arguments, read on entry and on return).

The script talks to `http://127.0.0.1:2346/mcp`, not `localhost`: on Windows `localhost`
tries IPv6 first and every call then waits ~2 s for that to fail (2074 ms vs 33 ms per call
measured here; arming 331 breakpoints takes 1.8 s at 33 ms).  Other MCP clients in this repo
that use `localhost` pay the same 2 s per call.

The MCP server has one session: starting the script ends any other client's session
(another Claude session's trap script, for example), so check nobody else is driving
DuckStation first.

## Checking struct layouts live (`tools/duckstation/live_probe.py`)

The matching agents propose struct layouts from the code alone: credits at `0x80039CA4`,
equipped part ids at `0x80031A58`, the pad state at `0x801A2550`, and so on.  This script
checks those layouts against the running game, one action at a time.  It only reads
RAM: no breakpoints, no pauses.

```
python tools/duckstation/live_probe.py --list            # the steps and what each one expects
python tools/duckstation/live_probe.py --check           # MCP up? every region readable?
python tools/duckstation/live_probe.py                   # all steps, in order (s skips one)
python tools/duckstation/live_probe.py --step buy --step head
python tools/duckstation/live_probe.py --show credits    # a region's fields as they are now
```

- **Diff steps** (`buy`, `head`, `colour`, `rename`, `mail`, `padmap`):
  1. Go to the screen the step names and press Enter.
  2. Do the one thing it asks (buy one part, swap the head, ...), then press Enter again.
  3. Answer its question if it has one: the price paid, or the name you typed.

  It prints every field that changed.  Bytes that no field covers are printed too
  (`+0x2C`); those are new findings.
- **Sample steps** (`idle`, `pad`):
  1. Press Enter.
  2. Switch to the game within 3 s.
  3. Hold the button it names (or keep your hands off) while it reads the regions over
     and over.

  Fields that move are printed per phase, along with the button's bit.
- Each expectation gets a verdict:
  - `CONFIRMED`;
  - `REFUTED`, together with what RAM did instead;
  - `UNCLEAR`, when a question was skipped.

  Verdicts, and every change no expectation covers (verdict `seen`), are appended to
  `re/match/live_probe.tsv`. Its columns are `step field addr expect before after verdict note timestamp`.

Steps, regions and expectations live in `tools/duckstation/probe_steps.json`.  Each
region notes which agent proposed it (`src`).  Add a region or a step there; the script
needs no change.  Only `CONFIRMED` layouts should move into `include/ac1`.  Test:
`python -m pytest tools/duckstation/test_live_probe.py`.

Like `coverage_tag.py`, it starts a new MCP session.  That ends any other client's
session, so don't run the two at the same time.

## Struct survey (`structsurvey.py`)

Matched sources declare the layouts they needed as local typedefs (agents may not edit
`include/ac1`).  `structsurvey.py` reads every `re/match/**/*.c`, lays each local
struct out with the MIPS rules (sizes of `include/ac1` types from their headers), and
finds the ones bound to a fixed address (`extern T D_X;`, `extern T *D_X;`,
`(T *)0x...` / `(T *)&D_X` casts, names from `symbols.tsv`).

```bash
python tools/match/structsurvey.py merge 0x801A2550   # one row per (offset, width): every name, file count
python tools/match/structsurvey.py fields 0x80039D18  # each file's declaration of one global (or type name)
python tools/match/structsurvey.py table -o FILE.md   # the survey tables in docs/STRUCT_SURVEY.md
python tools/match/structsurvey.py report             # long form, widest layout per address in full
```

`merge` is the consolidation view: a row marked `~` overlaps another named row of a
different width (two views of the same bytes, or a conflict to settle).  Computed
offsets are checked against the sources' own `+0xNN` comments (`!` in `fields`).
Its output is a starting point: settle names from the code before a layout goes into
`include/ac1`.

## Codegen probes

`probes/pNN_*.c` are A/B pairs of tiny functions, one per row of the "GCC 2.7.2
divergence families" part of the match-function skill table (register coloring,
address materialization, empty frames, epilogue fill, exit merging, operand order,
loops).  `python tools/match/probes/run_probes.py` compiles both variants through
`match_fn.py`, checks that B differs from (or equals) A as claimed and that each
variant's disassembly has the claimed shape; `tests/test_probes.py` runs it.
26/26 hold with the pinned toolchain (2026-10-19).

## Known cc1 problem

Both MinGW-built `cc1.exe` binaries tried (this worktree and another checkout)
crash with an access violation (exit 3221225477) on two one-trip loops:
`for (i = 0; i < 1; i++) p = &G;` and the same loop around
`v = (p->l | p->r) & p->cur;` with `p = &G` set before it.
`for (i = 0; i < 1; i++) s += G[i];` and ordinary loops compile fine.
match_fn.py reports it as `ERROR: cc1.exe failed`.  Not checked against the Linux
tarball (no WSL here); avoid the shape or cross-check on Linux.

## Matched functions

`re/match/manifest.tsv` lists every function that has reached MATCH (address,
overlay, source, size, extra args) and `re/match/check_all.sh` re-verifies all of
them in one go; `re/match/ovl201/` holds the sources.  Sizes in the manifest
include the delay slot after the final `jr ra`.

## Files

- `setup.sh` — toolchain bring-up (see Install)
- `patches/` — our fixes to vendored tools, applied by `setup.sh`
- `match_fn.py` — the oracle
- `include/ac1_types.h` — `u8/u16/u32/s8/s16/s32` for hand-written sources
- `include/ac1/ac1.h` — forwarder so a source can `#include "ac1/ac1.h"` and get the shared
  struct layouts and named globals in `<repo>/include/ac1` (see `include/ac1/README.md`);
  such rows add `--syms include/ac1/symbols.tsv --syms ac1_markup.tsv`
- `variants.py` — parallel batch of small C rewrites through the oracle's compile (Variant sweeps)
- `objdiff_prep.py` — objdiff project + per-function target/base objects (objdiff workstation)
- `ctx.py` — keeps `include/ac1/symbols.h` in step with `symbols.tsv`, writes the m2c
  context, and decompiles one function with it (`ctx.py m2c VADDR --unit 201`)
- `structsurvey.py` — the struct layouts matched sources declare locally, merged per
  global address or by shape (Struct survey)
- `tests/` — pytest suite, shell runner, the two sources
- `probes/` — A/B codegen probes behind the skill table, `run_probes.py`
- `vendor/` — gitignored toolchain (`cc1[.exe]`, `maspsx/`, `toolchain.json`, `VERSIONS.txt`, `cache/`)
- `.claude/skills/match-function/SKILL.md` — the Claude loop that uses this
