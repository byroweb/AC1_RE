# AC1 RE tool belt (`tools/`)

Accelerators for the Armored Core 1 reverse-engineering workflow (the Farsi
localization mod's own build scripts live in `mods/farsi/`).
Run `tools/ghidra/setup_re_tools.sh` once on a new machine to install/verify everything.

## Where your files live (environment variables)
Nothing in the repo points at a fixed machine path. Scripts find the repo root from
their own location and take your local files from these variables:

| variable | meaning | default |
| --- | --- | --- |
| `AC1_BIN` | your pristine `SLUS-01323` v1.1 image (2352-B sectors) | *(required by `extract_fdat.py`, `build_filemap.py`, the Farsi bake)* |
| `AC1_RE_ROOT` | repo root, for Ghidra scripts run from a copied script dir | parent of `ghidra_scripts/` |
| `AC1_GHIDRA_PROJECT_DIR` | directory holding `AC_1_USA_2048_RE.gpr` | repo root |
| `M2C_DIR` | your m2c checkout (venv at `.venv`, POSIX `bin/` or Windows `Scripts/` layout) | `~/Applications/m2c` |
| `GHIDRA_INSTALL_DIR` | unpacked Ghidra release (`ghidraRun[.bat]`, `support/analyzeHeadless[.bat]`) | *(unset → the Ubuntu snap paths)* |
| `GHIDRA_EXT_DIR` | Ghidra Extensions dir (where `ghidra_psx_ldr` goes) | `$GHIDRA_INSTALL_DIR/Ghidra/Extensions` if that is set, else the snap's `~/snap/ghidra/…` user dir |
| `MIPS_PREFIX` | cross-toolchain prefix for `gcc`/`objdump`/`objcopy`/`nm`/`as`/`ld` | auto: `mipsel-linux-gnu-` (apt) if on PATH, else `mipsel-none-elf-` (PCSX-Redux prebuilt) |

The shell scripts share this resolution via `tools/ghidra/re_env.sh`; the Python
tools (`tools/re/disasm_batch.py`, `mods/farsi/build_rtl_patch.py`) apply the same rules.

### Windows note
Everything here runs under **Git Bash** (MSYS2); PowerShell/cmd are not supported.
- Toolchain: the PCSX-Redux prebuilt `mipsel-none-elf-*` GCC is picked up unrenamed
  once its `bin` is on PATH (or set `MIPS_PREFIX=mipsel-none-elf-` explicitly).
- Python: scripts call `python3` where it exists and fall back to `python`. The
  Microsoft Store `python3` stub shadows a real install — either disable it in
  *App execution aliases* or make a `python3.exe` copy next to `python.exe`.
- Ghidra: set `GHIDRA_INSTALL_DIR` to the unpacked zip; the install-wide
  Extensions dir is `<install>\Ghidra\Extensions`, while Ghidra's per-user dir
  (settings, per-user extensions) lives at `%APPDATA%\ghidra\ghidra_<ver>_PUBLIC`.
- `ghidra_restart.sh` uses PowerShell `Stop-Process` instead of `pkill` and
  launches `ghidraRun.bat`; `ghidra_overlay.sh` runs `analyzeHeadless.bat`.

## `duckstation/` — save states
- **`states.py`** — the save-state **library**: `archive <slot> <id>` copies a
  DuckStation slot file into a directory outside the repo (`AC1_STATES_DIR`,
  default `~/dev/ac1_states`) under a stable ID, extracts the thumbnail, reads
  mission/stage/flags from RAM and appends a row to the committed manifest
  `disc_map/states/STATES.tsv`; `stage <id> <slot>` puts one back; `check`
  verifies sha256s; `list` queries. Slots are scratch, **docs cite IDs** — see
  `disc_map/states/README.md`. Test: `python tools/duckstation/states_test.py`.
- **`radio_line.py`** — plays one XA radio line in the running game through
  DuckStation's MCP server (sadnescity fork, port 2346) and measures what came out:
  stages a library state into a scratch slot with its media path rewritten to a
  disc copy's `.cue`, boots the copy, calls the game's XA line player
  `FUN_8004DB18(id)` from a breakpoint on `0x8005804C` with every register saved
  and restored, records with the `capture` tool and reports the normalised
  cross-correlation of the capture against the dubbed take and the pristine channel
  decodes (`docs/AUDIO_PIPELINE.md` §6). `--clip` takes a `voice_manifest.tsv` id,
  `--id` a raw `file_index << 4 | channel`; urllib only for the emulator part,
  `tools/audio/.venv` for the analysis (`--analyse-only` runs it on an existing MP4).
  ```
  tools/audio/.venv/Scripts/python tools/duckstation/radio_line.py --clip TALK00_ch12_c000 \
      --disc ~/dev/ac1_dub.cue --capture out/dub.mp4 --take take.wav \
      --shutdown --boot-after "${AC1_BIN%.bin}.cue" --load-after 3 --delete-slot
  ```
- **`coverage_tag.py`** — tags open functions with the screen they run on while you
  play: execute breakpoints through the MCP server, overlay-aware, labels typed in the
  terminal; writes `re/match/coverage.tsv` and `hint` rows in `re/match/hints.tsv`.
  See `tools/match/README.md` (Coverage tagging from play). Test:
  `python -m pytest tools/duckstation/test_coverage_tag.py`.
- **`savestate.py`** — offline DUCCS reader (header, RAM by PS1 address, VRAM,
  thumbnail, frame counter); `docs/SAVESTATE_FORMAT.md`. Needs a zstd decoder:
  `pip install zstandard` or the `zstd` CLI (neither ships with Windows).
- Extra variables: `AC1_STATES_DIR` (library), `AC1_DS_DIR` (DuckStation data dir,
  auto-detected from Documents/DuckStation or `~/.local/share/duckstation`),
  `AC1_DS_EXE` (emulator exe, for the build string). `AC1_BIN` supplies the EXE
  anchor + disc identity.

## `plantview/` — live "plant view" of the mission loop
A factory-floor style dashboard of the mission lifecycle (LOAD → RUNNING →
OBJECTIVES_COMPLETE → ENDING → EXIT), the entity table (spawn / update / collision /
destroy with per-station counts) and the per-frame pipeline, drawn from
`tools/plantview/stations.json` (every address with its CONFIRMED / HYPOTHESIS label
from the docs). `poller.py` samples RAM live through the DuckStation MCP `read_memory`
path or offline from save states via `savestate.py`, and serves JSON + SSE on
127.0.0.1; `index.html` is a single self-contained canvas page with a replay scrubber.
See [`plantview/README.md`](plantview/README.md) for running it and adding a station.
```
python tools/plantview/poller.py --source states --open     # replay the state library
python tools/plantview/poller.py --source live --open       # DuckStation, port 2346
```

## `sceneview/` — AC1 levels in the browser, PS1-style, with knobs
A WebGL2 page that draws a mission's level with its real texels, taken from a save
state's VRAM and sampled like the PS1 GPU does. Walk a pilot through it with sliders
for camera distance, an AC1-style camera that drops under low ceilings, texture
density, texture LOD, floor subdivision and draw distance, plus toggles for the PS1
quirks (320×240, affine UVs, vertex snap, 15-bit dither) and a texel-density map. It
was built to judge how an on-foot mode would look at pilot scale. See
[`sceneview/README.md`](sceneview/README.md).
```
python tools/sceneview/server.py      # http://127.0.0.1:8771/
```

## `setup_re_tools.sh`
Idempotent installer + health report. Sets up m2c, checks the mipsel toolchain,
installs/verifies the Ghidra PSX loader + PSYQ signatures, and confirms the
overlay-import script. Safe to re-run; only fetches what's missing.

## `overlay2c.sh <hex_addr> <len> [binfile] [base]`  — the daily workhorse
Decompiles a slice of MIPS to C via `objdump` + **m2c** (target `mips-gcc-c`: the
retail code is GCC 2.7.2, see *Byte-matching toolchain* below — the old `mips-ido-c`
target produced IDO-style output for a compiler the game never used). The name/menu
code lives in a runtime overlay Ghidra's base project doesn't cover, so this turns a
live address range into readable C in one step.
```
tools/ghidra/overlay2c.sh 0x80082190 0x1b0          # decompile the name-input handler
```
- Default binfile = `overlay_80050000.bin` (RAM dump), base = `0x80050000`.
- To decompile freshly-patched live code, dump RAM first (DuckStation
  `read_memory` → file) and pass it as `binfile` with the right `base`.
- Internals: `objdump2m2c.py` converts objdump output to m2c GAS asm (branch
  targets → `.L` labels, `jal` → `func_<addr>`, `$`-prefixed regs). Out-of-slice
  branch targets get trailing stub labels + a WARNING (means `len` was too small
  — grow it to capture the whole function).
- m2c lives at your m2c install (e.g. `~/Applications/m2c`; venv at `.venv`).

**Validated** against hand-RE'd routines (cursor.update original `n*16+8`, the
name handler = `farsi_name_input.c`, `box_init` alloc `0x800501bc` + `+0x74`
update method). **Two caveats found doing so:**
- `overlay_80050000.bin` is a Jun-6 RAM dump → STALE for anything baked Jun-7
  (cursor RTL `0x80083a28`, `boxhook`/`cursext` are original/zeros in it), and it
  is not on this PC. The **current mission-overlay image** is the full 2 MB RAM dump
  `M30-ctrl-01__20261017.ram.bin` in the save-state library (`$AC1_STATES_DIR`,
  default `~/dev/ac1_states`): pristine disc, mission 30 live, so FDAT overlay 202
  is resident at `0x8004ADA0` and the objobj at `0x801C4B40`. Pass it with
  `base 0x80000000`:
  ```
  tools/ghidra/overlay2c.sh 0x8004F1A8 0x200 "$AC1_STATES_DIR/M30-ctrl-01__20261017.ram.bin" 0x80000000
  ```
  The same image can be regenerated from the archived state offline
  (`savestate.py <state> --dump-ram out.bin`). For freshly baked code, re-dump
  live RAM (DuckStation `read_memory` path=) on the relevant screen instead.
- m2c can't resolve `jr` **switch/jump tables** ("Unable to determine jump
  table") — decode a sub-range that avoids the dispatcher, or read those in
  Ghidra/live disasm.

## `psyq_labels.py` — PsyQ library names from the v1.0 decomp
SLUS-01323 and SCUS-94182 share byte-identical code (`docs/REFERENCE.md` §1), so the
signature-identified PsyQ 3.7 library list in the CC0
[Wyrelade/Armored-Core-Decomp](https://github.com/Wyrelade/Armored-Core-Decomp) repo
(`configs/USA/psyq_funcs.txt`, 528 functions in `0x80016EA4`–`0x80030368` + `AddDrv`/`DelDrv`)
applies to our exe address for address. The script **downloads that file at run time**
(nothing vendored), emits a label list and compares it with `ac1_markup.tsv`:
```
python tools/ghidra/psyq_labels.py --out psyq_labels.txt      # Ghidra ImportSymbolsScript.py format
python tools/ghidra/psyq_labels.py --format tsv --named-only  # ac1_markup-style F rows
python tools/ghidra/psyq_labels.py -v                         # list every gained / corrected name
python tools/ghidra/psyq_labels.py --merged ac1_markup_psyq.tsv   # markup copy with names applied
```
Import in Ghidra: Script Manager → `ImportSymbolsScript.py` → pick the `.txt`
(`name 0xADDR f` per line creates the function + label). `ac1_markup_pristine.tsv` is
never written. `--source <path>` works offline from a saved copy.

## `wyrelade_layout.py` — every code unit's layout + v1.0 hash, re-verified on your disc
Same source and same rule (facts only, fetched at run time): the Wyrelade splat configs
(`configs/USA/*.yaml`, `mission_overlays.txt`) give, for the exe and all 62 `FDAT.T` code
entries, the load address, size, `.rodata`/`.text`/`.data` boundaries, the v1.0 SHA-1 and
which mission overlay is a byte-identical alias (168 = 166). The committed table is
`overlays/CODE_UNITS.tsv`.
```
python tools/ghidra/wyrelade_layout.py --out overlays/CODE_UNITS.tsv   # regenerate the table
python tools/ghidra/wyrelade_layout.py --check                          # carve every unit from $AC1_BIN,
                                                                        # compare SHA-1 with v1.0
python tools/ghidra/wyrelade_layout.py --md                             # Markdown table
```
`--check` is the reproducible proof behind the "v1.1 code == v1.0 code" note in
`docs/REFERENCE.md` (2026-10-18: 63 identical, 0 different). The `text_va`/`text_end_va`
columns are the ranges to feed `disasm_batch.py` / `overlay2c.sh` for each overlay.

What that repo does **not** give us: its `configs/USA/sym.fdat*.txt` symbol files are
empty and `sym.main.txt` holds only the PsyQ names already covered by `psyq_labels.py`;
its game-function names live in its C source, which we do not copy.

## Byte-matching toolchain (retail compiler, optional)
The retail code is **PsyQ 4.0 `CC1PSX` = GCC 2.7.2 (SN32 3.7.0002)** assembled by
**ASPSX 2.56** (identification: the Wyrelade repo's compiler probe; details in
`docs/REFERENCE.md` → *Retail compiler*). The modern `mipsel` GCC used for the patches
will never reproduce retail bytes; for matching experiments you need the period compiler:

- **Compiler:** [decompals/old-gcc](https://github.com/decompals/old-gcc) builds
  `gcc-2.7.2-psx` in Docker (`cd 2.7.2-psx && docker build .`, or take the prebuilt
  release tarball from that repo's Releases page). You get `cc1` (and `cpp`); the SN
  build is bit-identical to PsyQ 4.0's `CC1PSX.EXE`, so a real PsyQ 4.0 install works too.
- **Assembler:** [mkst/maspsx](https://github.com/mkst/maspsx) (`pip install` from the
  checkout, or run `maspsx.py`) turns `cc1`'s `-mgas` output into what ASPSX 2.56 would
  have emitted (`$at` expansion of `lw d,sym(reg)`, no div-by-zero traps), then feeds a
  modern `mipsel-*-as`; the link step is plain GNU `ld`.
- **The exact retail `cc1` line** (same for the exe and every overlay):
  ```
  cpp -lang-c -undef -D__GNUC__=2 -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D__CHAR_UNSIGNED__ -D_LANGUAGE_C -DLANGUAGE_C src.c     | cc1 -O2 -mips1 -mcpu=3000 -w -funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return -fcommon -msoft-float -mgas -fgnu-linker -gcoff -G0 -quiet -o out.s
  maspsx.py --aspsx-version=2.56 out.s | mipsel-none-elf-as -EL -march=r3000 -mabi=32 -no-pad-sections -O1 -G0 -o out.o
  ```
  The `cc1` flags are the verified retail set; the `cpp` defines and the `as` line are
  the conventional PsyQ-decomp ones (same recipe the Wyrelade `tools/build_ac.py` uses)
  and may need trimming to taste. Compare with `objdiff` or `asm-differ` against the
  bytes carved from your own disc (`disc_map/overlays/ovl201_front_end.bin`, base
  `0x8004ADA0`).

## `ghidra_restart.sh`
Kills Ghidra (GUI + decompiler + headless), clears stale `.lock`/`.lock~` that
block reopening, and relaunches Ghidra (`$GHIDRA_INSTALL_DIR`, else the snap) with `AC_1_USA_2048_RE.gpr`. Does
NOT kill the GhidraMCP python bridge. After the window opens, double-click the
program so GhidraMCP serves on `:8080` (health: `curl :8080/list_functions`).

## `resurrect.sh [--go] [delay]`
Restarts THIS Claude Code session and resumes the same conversation (full
context) — used after building new MCP servers / Ghidra plugins / deferred tools
that need a fresh Claude to load. DRY-RUN by default; `--go` arms a detached
supervisor that waits `delay`s, opens a new terminal running
`claude --resume $CLAUDE_CODE_SESSION_ID` in this repo, then kills the old
session. Needs `$CLAUDE_CODE_SESSION_ID` (Claude Code exports it).

## PSYQ signatures (auto-name SDK functions) — Ghidra, GUI step
`ghidra_psx_ldr` (installed in the Ghidra Extensions dir, see `GHIDRA_EXT_DIR`) ships PSYQ signature DBs
`psyq260…psyq470.gdt`. To label `libgpu`/`libgte`/`libcd`/`libapi` etc. in the
base exe: CodeBrowser → `Edit ▸ Options ▸ Program Information ▸ PsyQ Version`,
set the version, then re-run Auto Analysis with the PsyQ Signatures analyzer on.
AC1 is a Psy-Q title, so this turns many unnamed `FUN_*` into named SDK calls.

## Overlay → Ghidra (static decompile of overlay)
`ghidra_scripts/ImportAC1Overlay.py` maps `overlay_80050000.bin` in at
`0x80050000` and disassembles known entries, so the overlay becomes decompilable
via Ghidra + the MCP. Run it from the Ghidra Script Manager. (For one-off
functions, `overlay2c.sh` is faster.)
```
```

## jPSXdec (disc indexer + XA / STR decoder)
The disc map (`disc_map/disc_files.json`) and the voice pipeline both use
[jPSXdec](https://github.com/m35/jpsxdec) (Java; v2.x runs on the Temurin JDK 21 at
`JAVA_HOME`). Install once:

1. Download `jpsxdec_v2.1-beta.zip` from the official release page
   <https://github.com/m35/jpsxdec/releases/tag/v2.1> into its own empty folder
   (e.g. `~/dev/_dl/jpsxdec_v2.1/`).
2. Verify it against the release's digest (shown by
   `gh api repos/m35/jpsxdec/releases/latest --jq '.assets[0].digest'`):
   `sha256sum jpsxdec_v2.1-beta.zip` → `e11787a2…cd45b` for v2.1.
3. Unpack to `~/dev/jpsxdec/` (gives `~/dev/jpsxdec/jpsxdec_v2.1-beta/jpsxdec.jar`,
   plus `doc/jPSXdec-manual.pdf`). The audio scripts find it there; elsewhere set
   `JPSXDEC_JAR=/path/to/jpsxdec.jar`.

Handy commands (jPSXdec writes `index.log`/`save.log`/`debug00.log` into the cwd —
run it from a scratch dir, as the audio scripts do):
```
java -jar "$JPSXDEC_JAR" -f "$AC1_BIN" -x ac1.idx            # index the disc (~6 s)
java -jar "$JPSXDEC_JAR" -x ac1.idx -i "GG/BGM/TALK00.XA[3]" -af wav -dir out   # one XA channel
java -jar "$JPSXDEC_JAR" -x ac1.idx -i "GG/STR/ACED1.STR[0.0]" -af wav -dir out  # a video's audio
```
`tools/extract/build_filemap.py` turns such an index into `disc_files.json`.

## `ac1mod/` — disc browser + model/mission viewer (`tools/ac1mod/`)
PyQt6 GUI and headless CLI for browsing the disc index (TIM/text/hex/XA previews and
replacement import) and viewing PA##.T geometry, assembled mission levels with
spawns, MIS.T preview TIMs and memory-card emblems. Imported from the former AC1mod
repo; see [`ac1mod/README.md`](ac1mod/README.md).
```
tools/ac1mod/setup.sh          # venv at tools/ac1mod/.venv (PyQt6, Pillow, numpy, pygame)
tools/ac1mod/run_ac1mod.sh     # GUI
tools/ac1mod/.venv/Scripts/python.exe tools/ac1mod/ac1mod_cli.py --project P.ac1mod level 1 -o m01.png
```
It finds jPSXdec and Java the same way as the audio scripts (section above). It
decodes PA geometry, `.T` containers and XA audio with `tools/pa/pa_obj.py`,
`tools/extract/{extract_t,t_repack}.py` and `tools/audio/xa_adpcm.py`, and reads
mission levels and spawns with `tools/mission/{mission_geom,mission_parse}.py`; its
`core/` modules are adapters over those, so format fixes go in those directories.

## `audio/` — spoken voice lines → manifest → dub (`tools/audio/`)
Extracts the TALK XA channels and STR audio tracks with jPSXdec, splits them into
clips, transcribes them with faster-whisper (CPU, language detected per clip —
the disc has Japanese lines, which get Japanese text plus an English gloss), gates
out ambience / sound effects, clusters speakers, matches the English text against
the MIS.T corpus and writes `disc_map/voice_manifest.tsv` plus a local review
page. Full write-up: [`docs/AUDIO_PIPELINE.md`](../docs/AUDIO_PIPELINE.md).
```
tools/audio/setup_audio.sh                                   # venv + deps (once)
tools/audio/.venv/Scripts/python tools/audio/run_all.py      # end to end
tools/audio/.venv/Scripts/python tools/audio/audio_test.py   # unit tests
```
Needs `AC1_BIN`, `JAVA_HOME` (JDK 21) and jPSXdec (above). Decoded audio stays in
`disc_map/extracted/AUDIO/` (gitignored); committed are the manifest and
`disc_map/voice_reclaimable.tsv` — the Japanese / non-speech XA slots that can be
re-filled (`reclaimable.py`, with a static reference scan by `xa_refs.py`; docs §7).

**Re-injection** (`reinject_xa.py`, docs §6): writes a replacement WAV take into
one TALK channel of a *copy* of the disc, driven by a manifest row. Encodes to
4-bit XA-ADPCM with `xa_adpcm.py` (pure numpy: codec, Form 2 sector packing,
CD-ROM EDC), rewrites exactly the clip's sectors keeping the original subheaders,
refuses takes longer than the clip's sector budget, and can verify the result by
decoding the patched stream with jPSXdec.
```
tools/audio/.venv/Scripts/python tools/audio/reinject_xa.py --clip TALK05_ch00_c000 \
    --wav take.wav --image ~/dev/ac1_dub.bin --verify-jpsxdec     # copies $AC1_BIN -> --image first
tools/audio/.venv/Scripts/python tools/audio/reinject_test.py   # codec / EDC / packing tests, no disc
```
`--image` must never be the pristine image (the script refuses it); `--dry-run`
encodes and reports without writing, `--trim` cuts an over-long take.

