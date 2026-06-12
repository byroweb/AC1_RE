#!/usr/bin/env bash
# resurrect.sh — stop THIS Claude Code session and, after a short delay,
# relaunch a fresh Claude that RESUMES the same conversation (full context).
#
# Primary use: after building new MCP servers / Ghidra plugins / deferred tools,
# Claude must be restarted to load them. This automates the hand-off: a detached
# supervisor waits, opens a new terminal running `claude --resume <session>` in
# the repo, then kills the old (current) Claude process.
#
# Because it kills the current session, it is DRY-RUN by default. Pass --go to
# actually do it. The resumed session keeps the entire conversation, so work
# continues seamlessly (I just pick up where I left off).
#
# Usage:
#   tools/ghidra/resurrect.sh                 # dry-run: print the plan, change nothing
#   tools/ghidra/resurrect.sh --go [delay]    # execute; delay seconds before relaunch (default 8)
#
# Env: CLAUDE_CODE_SESSION_ID must be set (Claude Code exports it).
set -uo pipefail

GO=0; DELAY=8
for a in "$@"; do
  case "$a" in
    --go) GO=1 ;;
    [0-9]*) DELAY="$a" ;;
  esac
done

SID="${CLAUDE_CODE_SESSION_ID:-}"
CWD="$(pwd)"
CLAUDE_BIN="$(command -v claude || echo /home/byron/.local/bin/claude)"

if [ -z "$SID" ]; then
  echo "[resurrect] ERROR: CLAUDE_CODE_SESSION_ID not set; cannot resume."; exit 1
fi

# Walk parents to find the live Claude (node) process to terminate.
find_claude_pid() {
  local pid=$$ ppid cmd
  for _ in $(seq 1 12); do
    ppid=$(awk '/^PPid:/{print $2}' "/proc/$pid/status" 2>/dev/null)
    [ -z "$ppid" ] || [ "$ppid" = "0" ] && break
    cmd=$(tr '\0' ' ' < "/proc/$ppid/cmdline" 2>/dev/null)
    comm=$(cat "/proc/$ppid/comm" 2>/dev/null)
    if [ "$comm" = "claude" ] || echo "$cmd" | grep -qE '(^|/|[[:space:]])claude([[:space:]]|$)|share/claude/versions/.*/cli'; then
      echo "$ppid"; return 0
    fi
    pid=$ppid
  done
  return 1
}

# Pick an available terminal emulator.
pick_term() {
  for t in gnome-terminal konsole xfce4-terminal kitty alacritty xterm; do
    command -v "$t" >/dev/null 2>&1 && { echo "$t"; return; }
  done
}
TERM_EMU="$(pick_term)"
OLD_PID="$(find_claude_pid || true)"

RESUME_CMD="cd '$CWD' && exec '$CLAUDE_BIN' --resume '$SID'"

echo "[resurrect] session : $SID"
echo "[resurrect] cwd     : $CWD"
echo "[resurrect] terminal: ${TERM_EMU:-<none found>}"
echo "[resurrect] old PID : ${OLD_PID:-<not found>}"
echo "[resurrect] delay   : ${DELAY}s"
echo "[resurrect] relaunch: claude --resume $SID"

if [ -z "$TERM_EMU" ]; then
  echo "[resurrect] ERROR: no terminal emulator found; aborting."; exit 1
fi

# Build the new-terminal invocation per emulator.
case "$TERM_EMU" in
  gnome-terminal) NEWTERM=(gnome-terminal --working-directory="$CWD" -- bash -lc "$RESUME_CMD") ;;
  konsole)        NEWTERM=(konsole --workdir "$CWD" -e bash -lc "$RESUME_CMD") ;;
  xfce4-terminal) NEWTERM=(xfce4-terminal --working-directory="$CWD" -x bash -lc "$RESUME_CMD") ;;
  kitty)          NEWTERM=(kitty --directory "$CWD" bash -lc "$RESUME_CMD") ;;
  alacritty)      NEWTERM=(alacritty --working-directory "$CWD" -e bash -lc "$RESUME_CMD") ;;
  xterm)          NEWTERM=(xterm -e bash -lc "$RESUME_CMD") ;;
esac

if [ "$GO" -ne 1 ]; then
  echo
  echo "[resurrect] DRY-RUN (no changes). Re-run with --go to execute:"
  printf '   spawn: %q ' "${NEWTERM[@]}"; echo
  echo "   then : kill ${OLD_PID:-<old claude pid>}"
  exit 0
fi

# Detached supervisor: wait, open the new terminal, then kill the old session.
SUP=$(mktemp /tmp/resurrect_sup.XXXX.sh)
{
  echo '#!/usr/bin/env bash'
  echo "sleep $DELAY"
  printf '%q ' "${NEWTERM[@]}"; echo '&'
  echo 'sleep 3'
  [ -n "$OLD_PID" ] && echo "kill $OLD_PID 2>/dev/null; sleep 2; kill -9 $OLD_PID 2>/dev/null"
  echo "rm -f '$SUP'"
} > "$SUP"
chmod +x "$SUP"
setsid nohup "$SUP" >/tmp/resurrect.log 2>&1 &
disown 2>/dev/null || true
echo "[resurrect] supervisor armed (log: /tmp/resurrect.log). New Claude in ${DELAY}s."
