#!/usr/bin/env bash
# re_env.sh — shared tool resolution for the tools/ghidra scripts.
#
# Source it (never execute it):   . "$(dirname "$0")/re_env.sh"
#
# Lets the same scripts run on Linux (apt toolchain, snap Ghidra) and on Windows
# under Git Bash / MSYS2 (PCSX-Redux mipsel-none-elf toolchain, zip Ghidra)
# without renaming binaries or adding shims. Every resolver prints ONE value and
# always returns 0 (callers run under `set -e`); when nothing is found it prints
# the Linux default so the eventual "command not found" names the usual tool.
#
# Environment variables honoured (see tools/README.md):
#   M2C_DIR             m2c checkout with a .venv            (default ~/Applications/m2c)
#   MIPS_PREFIX         cross-toolchain prefix               (default: auto, see re_mips_prefix)
#   GHIDRA_INSTALL_DIR  unpacked Ghidra release dir          (default: snap paths)

re_is_windows() {
  case "$(uname -s 2>/dev/null)" in MINGW*|MSYS*|CYGWIN*) return 0 ;; *) return 1 ;; esac
}

# Python 3 interpreter name: python3 where it exists, else python (Windows
# installs usually ship only python.exe).
re_python() {
  if command -v python3 >/dev/null 2>&1; then echo python3; else echo python; fi
}

# m2c venv interpreter: $M2C_DIR/.venv/bin/python (POSIX layout) or
# $M2C_DIR/.venv/Scripts/python.exe (Windows layout), whichever exists.
re_m2c_python() {
  local m2c="${1:-${M2C_DIR:-$HOME/Applications/m2c}}" p
  for p in "$m2c/.venv/bin/python" "$m2c/.venv/Scripts/python.exe"; do
    if [ -x "$p" ]; then echo "$p"; return 0; fi
  done
  echo "$m2c/.venv/bin/python"
}

# MIPS cross-toolchain prefix. $MIPS_PREFIX wins; otherwise the first prefix
# whose gcc or objdump is on PATH: mipsel-linux-gnu- (Debian/Ubuntu apt
# packages) then mipsel-none-elf- (PCSX-Redux prebuilt toolchain, Windows).
re_mips_prefix() {
  if [ -n "${MIPS_PREFIX:-}" ]; then echo "$MIPS_PREFIX"; return 0; fi
  local p
  for p in mipsel-linux-gnu- mipsel-none-elf-; do
    if command -v "${p}gcc" >/dev/null 2>&1 || command -v "${p}objdump" >/dev/null 2>&1; then
      echo "$p"; return 0
    fi
  done
  echo mipsel-linux-gnu-
}

# Ghidra GUI launcher: $GHIDRA_INSTALL_DIR/ghidraRun (.bat on Windows), else the
# Ubuntu snap wrapper.
re_ghidra_run() {
  local d="${GHIDRA_INSTALL_DIR:-}"
  if [ -n "$d" ]; then
    if re_is_windows && [ -f "$d/ghidraRun.bat" ]; then echo "$d/ghidraRun.bat"; return 0; fi
    if [ -f "$d/ghidraRun" ]; then echo "$d/ghidraRun"; return 0; fi
  fi
  echo /snap/bin/ghidra
}

# Ghidra headless analyzer: $GHIDRA_INSTALL_DIR/support/analyzeHeadless (.bat on
# Windows), else the snap's 12.0 build.
re_ghidra_headless() {
  local d="${GHIDRA_INSTALL_DIR:-}"
  if [ -n "$d" ]; then
    if re_is_windows && [ -f "$d/support/analyzeHeadless.bat" ]; then echo "$d/support/analyzeHeadless.bat"; return 0; fi
    if [ -f "$d/support/analyzeHeadless" ]; then echo "$d/support/analyzeHeadless"; return 0; fi
  fi
  echo /snap/ghidra/current/ghidra_12.0_PUBLIC/support/analyzeHeadless
}
