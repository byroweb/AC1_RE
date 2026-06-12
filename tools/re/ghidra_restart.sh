#!/usr/bin/env bash
# ghidra_restart.sh — kill any running Ghidra, clear stale project locks, and
# relaunch Ghidra with the AC1 RE project open.
#
# Why: Ghidra (snap 12.0, HELD) sometimes wedges or leaves a stale .lock/.lock~
# after an unclean exit, which blocks reopening the project ("project is
# locked"). This does the kill + lock-clear + relaunch in one shot.
#
# After the project window opens, double-click the program (CodeBrowser) to make
# the GhidraMCP plugin start its server on 127.0.0.1:8080. Health-check:
#   curl -s --max-time 5 http://127.0.0.1:8080/list_functions
#
# Usage: tools/re/ghidra_restart.sh
set -uo pipefail

PROJ_DIR="/home/byron/Desktop/AC_1_USA_RE"
GPR="$PROJ_DIR/AC_1_USA_2048_RE.gpr"     # active project (the one used with MCP)
GHIDRA="/snap/bin/ghidra"

echo "[ghidra_restart] killing Ghidra processes..."
# GUI (GhidraRun), the decompile helper, and any analyzeHeadless
pkill -f 'ghidra.GhidraRun'            2>/dev/null && echo "  killed GhidraRun"
pkill -f 'Features/Decompiler/.*/decompile' 2>/dev/null && echo "  killed decompiler"
pkill -f 'ghidra.*analyzeHeadless'     2>/dev/null && echo "  killed headless"
# NB: do NOT kill the GhidraMCP python bridge (bridge_mcp_ghidra.py) — it just
# reconnects to the plugin once a program is reopened.
sleep 2

echo "[ghidra_restart] clearing stale project locks..."
rm -f "$PROJ_DIR"/AC_1_USA_2048_RE.lock "$PROJ_DIR"/AC_1_USA_2048_RE.lock~ 2>/dev/null
# also clear any inner-project rep lock if present
find "$PROJ_DIR/AC_1_USA_2048_RE.rep" -name '*.lock*' -delete 2>/dev/null

if [ ! -f "$GHIDRA" ]; then
    echo "[ghidra_restart] ERROR: $GHIDRA not found (snap installed?)"; exit 1
fi

echo "[ghidra_restart] launching Ghidra with project: $GPR"
setsid nohup "$GHIDRA" "$GPR" >/tmp/ghidra_launch.log 2>&1 &
disown 2>/dev/null || true
echo "[ghidra_restart] launched (log: /tmp/ghidra_launch.log)."
echo "[ghidra_restart] NEXT: double-click the program in the project window to"
echo "                 start GhidraMCP, then health-check :8080/list_functions."
