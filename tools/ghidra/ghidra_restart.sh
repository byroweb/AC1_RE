#!/usr/bin/env bash
# ghidra_restart.sh — kill any running Ghidra, clear stale project locks, and
# relaunch Ghidra with the AC1 RE project open.
#
# Why: Ghidra (snap 12.0 on Linux, zip install elsewhere) sometimes wedges or leaves a stale .lock/.lock~
# after an unclean exit, which blocks reopening the project ("project is
# locked"). This does the kill + lock-clear + relaunch in one shot.
#
# After the project window opens, double-click the program (CodeBrowser) to make
# the GhidraMCP plugin start its server on 127.0.0.1:8080. Health-check:
#   curl -s --max-time 5 http://127.0.0.1:8080/list_functions
#
# Ghidra is found via $GHIDRA_INSTALL_DIR (ghidraRun / ghidraRun.bat), falling
# back to the Ubuntu snap. Works on Linux and on Windows under Git Bash.
#
# Usage: tools/ghidra/ghidra_restart.sh
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
. "$REPO/tools/ghidra/re_env.sh"
PROJ_DIR="${AC1_GHIDRA_PROJECT_DIR:-$REPO}"   # where AC_1_USA_2048_RE.gpr lives (gitignored)
GPR="$PROJ_DIR/AC_1_USA_2048_RE.gpr"     # active project (the one used with MCP)
GHIDRA="$(re_ghidra_run)"
LOG="${TMPDIR:-/tmp}/ghidra_launch.log"

echo "[ghidra_restart] killing Ghidra processes..."
# GUI (GhidraRun), the decompile helper, and any analyzeHeadless.
# NB: do NOT kill the GhidraMCP python bridge (bridge_mcp_ghidra.py) — it just
# reconnects to the plugin once a program is reopened.
if re_is_windows; then
  # No pkill: kill the java/javaw whose command line runs GhidraRun or
  # AnalyzeHeadless (-match is case-insensitive), plus the decompile.exe helper
  # by name. Only java processes are considered so a shell whose command line
  # merely mentions the pattern is never hit.
  powershell -NoProfile -Command "
    Get-CimInstance Win32_Process |
      Where-Object { \$_.Name -eq 'decompile.exe' -or (
        (\$_.Name -eq 'java.exe' -or \$_.Name -eq 'javaw.exe') -and
        \$_.CommandLine -match 'ghidra\.GhidraRun|AnalyzeHeadless') } |
      ForEach-Object { Stop-Process -Id \$_.ProcessId -Force; Write-Host ('  killed ' + \$_.Name + ' pid ' + \$_.ProcessId) }"
else
  pkill -f 'ghidra.GhidraRun'            2>/dev/null && echo "  killed GhidraRun"
  pkill -f 'Features/Decompiler/.*/decompile' 2>/dev/null && echo "  killed decompiler"
  pkill -f 'ghidra.*analyzeHeadless'     2>/dev/null && echo "  killed headless"
fi
sleep 2

echo "[ghidra_restart] clearing stale project locks..."
rm -f "$PROJ_DIR"/AC_1_USA_2048_RE.lock "$PROJ_DIR"/AC_1_USA_2048_RE.lock~ 2>/dev/null
# also clear any inner-project rep lock if present
find "$PROJ_DIR/AC_1_USA_2048_RE.rep" -name '*.lock*' -delete 2>/dev/null

if [ ! -f "$GHIDRA" ]; then
    echo "[ghidra_restart] ERROR: $GHIDRA not found (set GHIDRA_INSTALL_DIR to the unpacked release, or install the snap)"; exit 1
fi

echo "[ghidra_restart] launching Ghidra with project: $GPR"
if re_is_windows; then
  # ghidraRun.bat backgrounds javaw itself (support/launch.bat bg) and returns.
  cmd //c "$(cygpath -w "$GHIDRA")" "$(cygpath -w "$GPR")" >"$LOG" 2>&1 \
    || { echo "[ghidra_restart] ERROR: launch failed; see $LOG"; exit 1; }
else
  setsid nohup "$GHIDRA" "$GPR" >"$LOG" 2>&1 &
  disown 2>/dev/null || true
fi
echo "[ghidra_restart] launched (log: $LOG)."
echo "[ghidra_restart] NEXT: double-click the program in the project window to"
echo "                 start GhidraMCP, then health-check :8080/list_functions."
