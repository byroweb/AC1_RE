#!/usr/bin/env bash
# setup_re_tools.sh — idempotent installer + health report for the AC1 RE tool belt.
#
# Brings up / verifies everything that speeds up the reversing workflow:
#   1. m2c       — MIPS asm -> C decompiler (drives tools/ghidra/overlay2c.sh)
#   2. toolchain — mipsel-*-gcc / objdump (compile patches, disassemble)
#   3. PSX loader+ PSYQ signatures (ghidra_psx_ldr) — auto-name libgpu/gte/cd/etc.
#   4. overlay   — Ghidra script to map the runtime overlay into the base project
#
# Safe to re-run; only fetches what's missing. Run from anywhere (Linux, or
# Windows under Git Bash — see tools/README.md for the env vars).
#   tools/ghidra/setup_re_tools.sh
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
. "$REPO/tools/ghidra/re_env.sh"
APPS="${APPS_DIR:-$HOME/Applications}"
M2C="${M2C_DIR:-$APPS/m2c}"
PY="$(re_python)"            # python3, or python where only that exists (Windows)
PFX="$(re_mips_prefix)"      # $MIPS_PREFIX, else mipsel-linux-gnu- / mipsel-none-elf-
# Ghidra Extensions dir: $GHIDRA_EXT_DIR; else the install-wide
# $GHIDRA_INSTALL_DIR/Ghidra/Extensions (zip install, any OS); else the snap's
# per-user dir.
if [ -n "${GHIDRA_EXT_DIR:-}" ]; then GEXT="$GHIDRA_EXT_DIR"
elif [ -n "${GHIDRA_INSTALL_DIR:-}" ]; then GEXT="$GHIDRA_INSTALL_DIR/Ghidra/Extensions"
else GEXT="$HOME/snap/ghidra/current/.config/ghidra/ghidra_12.0_PUBLIC/Extensions"; fi
ok(){ echo "  [ok]   $*"; }
add(){ echo "  [+]    $*"; }
warn(){ echo "  [warn] $*"; }

echo "== AC1 RE tool belt =="

# 1. m2c -------------------------------------------------------------------
echo "m2c (asm->C):"
if [ ! -d "$M2C/.git" ]; then
  add "cloning matt-kempster/m2c"
  git clone --depth 1 https://github.com/matt-kempster/m2c.git "$M2C" >/dev/null 2>&1
fi
M2C_PY="$(re_m2c_python "$M2C")"   # .venv/bin/python or .venv/Scripts/python.exe
if [ ! -x "$M2C_PY" ]; then
  add "creating venv + graphviz"
  "$PY" -m venv "$M2C/.venv" && M2C_PY="$(re_m2c_python "$M2C")" \
    && "$M2C_PY" -m pip install -q graphviz
fi
if "$M2C_PY" "$M2C/m2c.py" --help >/dev/null 2>&1; then
  ok "m2c runnable  (use: $REPO/tools/ghidra/overlay2c.sh <addr> <len>)"
else warn "m2c not runnable"; fi

# 2. toolchain -------------------------------------------------------------
echo "toolchain (prefix $PFX; override with MIPS_PREFIX=):"
for t in "${PFX}gcc" "${PFX}objdump"; do
  if command -v "$t" >/dev/null; then ok "$t"; else
    warn "$t MISSING — Linux: apt install gcc-mipsel-linux-gnu binutils-mipsel-linux-gnu;"
    warn "  Windows: PCSX-Redux prebuilt mipsel-none-elf toolchain on PATH (MIPS_PREFIX=mipsel-none-elf-)"
  fi
done

# 3. PSX loader + PSYQ signatures -----------------------------------------
echo "Ghidra PSX loader + PSYQ signatures ($GEXT):"
if [ -d "$GEXT/ghidra_psx_ldr" ]; then
  ngdt=$(ls "$GEXT/ghidra_psx_ldr/data"/psyq*.gdt 2>/dev/null | wc -l)
  ok "ghidra_psx_ldr installed ($ngdt PSYQ .gdt signature DBs)"
  echo "         APPLY: in CodeBrowser set Edit->Options->Program Information->PsyQ"
  echo "         Version, then re-run Auto Analysis (PsyQ Signatures analyzer ON)."
else
  add "downloading ghidra_psx_ldr (Ghidra 12.0 build)"
  mkdir -p "$APPS/ghidra_extensions" "$GEXT"
  URL="https://github.com/lab313ru/ghidra_psx_ldr/releases/download/2026.06.04/ghidra_12.0_PUBLIC_20260604_ghidra_psx_ldr.zip"
  curl -sL --max-time 90 -o "$APPS/ghidra_extensions/ghidra_psx_ldr.zip" "$URL" \
    && unzip -q "$APPS/ghidra_extensions/ghidra_psx_ldr.zip" -d "$GEXT" \
    && ok "installed (restart Ghidra: tools/ghidra/ghidra_restart.sh)"
fi

# 4. overlay import script -------------------------------------------------
echo "overlay -> Ghidra:"
if [ -f "$REPO/ghidra_scripts/ImportAC1Overlay.py" ]; then
  ok "ImportAC1Overlay.py present (run from Ghidra Script Manager; maps 0x80050000)"
else warn "ImportAC1Overlay.py missing"; fi

echo
echo "Helpers: overlay2c.sh (asm->C) | ghidra_restart.sh (kill+reopen) | resurrect.sh (reload Claude)"
echo "Done."
