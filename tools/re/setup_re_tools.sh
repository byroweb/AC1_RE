#!/usr/bin/env bash
# setup_re_tools.sh — idempotent installer + health report for the AC1 RE tool belt.
#
# Brings up / verifies everything that speeds up the reversing workflow:
#   1. m2c       — MIPS asm -> C decompiler (drives tools/re/overlay2c.sh)
#   2. toolchain — mipsel-linux-gnu-gcc / objdump (compile patches, disassemble)
#   3. PSX loader+ PSYQ signatures (ghidra_psx_ldr) — auto-name libgpu/gte/cd/etc.
#   4. overlay   — Ghidra script to map the runtime overlay into the base project
#
# Safe to re-run; only fetches what's missing. Run from anywhere.
#   tools/re/setup_re_tools.sh
set -uo pipefail

REPO="/home/byron/Desktop/AC_1_USA_RE"
APPS="/home/byron/Applications"
M2C="$APPS/m2c"
GEXT="/home/byron/snap/ghidra/current/.config/ghidra/ghidra_12.0_PUBLIC/Extensions"
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
if [ ! -x "$M2C/.venv/bin/python" ]; then
  add "creating venv + graphviz"
  python3 -m venv "$M2C/.venv" && "$M2C/.venv/bin/pip" install -q graphviz
fi
if "$M2C/.venv/bin/python" "$M2C/m2c.py" --help >/dev/null 2>&1; then
  ok "m2c runnable  (use: $REPO/tools/re/overlay2c.sh <addr> <len>)"
else warn "m2c not runnable"; fi

# 2. toolchain -------------------------------------------------------------
echo "toolchain:"
for t in mipsel-linux-gnu-gcc mipsel-linux-gnu-objdump; do
  if command -v "$t" >/dev/null; then ok "$t"; else warn "$t MISSING (apt install gcc-mipsel-linux-gnu binutils-mipsel-linux-gnu)"; fi
done

# 3. PSX loader + PSYQ signatures -----------------------------------------
echo "Ghidra PSX loader + PSYQ signatures:"
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
    && ok "installed (restart Ghidra: tools/re/ghidra_restart.sh)"
fi

# 4. overlay import script -------------------------------------------------
echo "overlay -> Ghidra:"
if [ -f "$REPO/ghidra_scripts/ImportAC1Overlay.py" ]; then
  ok "ImportAC1Overlay.py present (run from Ghidra Script Manager; maps 0x80050000)"
else warn "ImportAC1Overlay.py missing"; fi

echo
echo "Helpers: overlay2c.sh (asm->C) | ghidra_restart.sh (kill+reopen) | resurrect.sh (reload Claude)"
echo "Done."
