# AC1 RE tool belt (`tools/`)

Accelerators for the Armored Core 1 reverse-engineering / Farsi-patch workflow.
Run `tools/ghidra/setup_re_tools.sh` once on a new machine to install/verify everything.

## `setup_re_tools.sh`
Idempotent installer + health report. Sets up m2c, checks the mipsel toolchain,
installs/verifies the Ghidra PSX loader + PSYQ signatures, and confirms the
overlay-import script. Safe to re-run; only fetches what's missing.

## `overlay2c.sh <hex_addr> <len> [binfile] [base]`  — the daily workhorse
Decompiles a slice of MIPS to C via `objdump` + **m2c**. The name/menu code lives
in a runtime overlay Ghidra's base project doesn't cover, so this turns a live
address range into readable C in one step.
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
  (cursor RTL `0x80083a28`, `boxhook`/`cursext` are original/zeros in it). For
  current baked code, re-dump live RAM (DuckStation `read_memory` path=) on the
  relevant screen and pass it as `binfile`.
- m2c can't resolve `jr` **switch/jump tables** ("Unable to determine jump
  table") — decode a sub-range that avoids the dispatcher, or read those in
  Ghidra/live disasm.

## `ghidra_restart.sh`
Kills Ghidra (GUI + decompiler + headless), clears stale `.lock`/`.lock~` that
block reopening, and relaunches the snap Ghidra with `AC_1_USA_2048_RE.gpr`. Does
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
`ghidra_psx_ldr` (installed in the snap Extensions dir) ships PSYQ signature DBs
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
