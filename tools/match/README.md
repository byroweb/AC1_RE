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
64-bit.  No divergence from the Linux tarball has been seen on the functions
tried; if a function refuses to match for no visible reason, cross-check with the
official tarball (`setup.sh --wsl` once WSL is installed, or on a Linux box).
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
cc1 is never rebuilt for this (`--force` is the only thing that rebuilds cc1).

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
- `ctx.py` — keeps `include/ac1/symbols.h` in step with `symbols.tsv`, writes the m2c
  context, and decompiles one function with it (`ctx.py m2c VADDR --unit 201`)
- `tests/` — pytest suite, shell runner, the two sources
- `probes/` — A/B codegen probes behind the skill table, `run_probes.py`
- `vendor/` — gitignored toolchain (`cc1[.exe]`, `maspsx/`, `toolchain.json`, `VERSIONS.txt`, `cache/`)
- `.claude/skills/match-function/SKILL.md` — the Claude loop that uses this
