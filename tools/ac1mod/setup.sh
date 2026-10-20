#!/usr/bin/env bash
# setup.sh — one-time setup for AC1mod (Git Bash on Windows, or Linux).
#
#   tools/ac1mod/setup.sh            # create tools/ac1mod/.venv, install deps, health check
#   tools/ac1mod/setup.sh --check    # health check only
#
# Java + jPSXdec are NOT installed here; see tools/ac1mod/README.md § Requirements.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV="$HERE/.venv"
if [[ -x "$VENV/Scripts/python.exe" ]]; then PY="$VENV/Scripts/python.exe"; else PY="$VENV/bin/python"; fi

pick_python() {
  for c in python3 python py; do
    if command -v "$c" >/dev/null 2>&1 && "$c" -c 'import sys; sys.exit(0 if sys.version_info >= (3,10) else 1)' 2>/dev/null; then
      echo "$c"; return
    fi
  done
  echo "python3"
}

if [[ "${1:-}" != "--check" ]]; then
  if [[ ! -e "$PY" ]]; then
    echo "[setup] creating venv at $VENV"
    "$(pick_python)" -m venv "$VENV"
    if [[ -x "$VENV/Scripts/python.exe" ]]; then PY="$VENV/Scripts/python.exe"; else PY="$VENV/bin/python"; fi
  fi
  echo "[setup] installing requirements"
  "$PY" -m pip install --quiet --upgrade pip
  "$PY" -m pip install --quiet -r "$HERE/requirements.txt"
fi

echo "[check] python : $("$PY" --version 2>&1)  ($PY)"
"$PY" - <<'EOF'
import importlib
for m in ("PyQt6.QtCore", "PIL", "numpy", "pygame"):
    try:
        mod = importlib.import_module(m)
        v = getattr(mod, "PYQT_VERSION_STR", None) or getattr(mod, "__version__", "?")
        print(f"[check] {m:<14} {v}")
    except Exception as e:
        print(f"[check] {m:<14} MISSING ({e})")
EOF
# The raster/XA hot loops compile on first use with $CC / gcc / clang; without one
# AC1mod falls back to numpy / pure Python (slower, same output).
CC_FOUND="${CC:-$(command -v gcc || command -v clang || command -v cc || true)}"
if [[ -n "$CC_FOUND" ]]; then echo "[check] host cc : $CC_FOUND"; else echo "[check] host cc : none (numpy fallback)"; fi
JAVA="${JAVA_HOME:+$JAVA_HOME/bin/java}"; JAVA="${JAVA:-$(command -v java || true)}"
if [[ -n "$JAVA" && -e "$JAVA" || -n "$JAVA" && -e "$JAVA.exe" ]]; then
  echo "[check] java   : $("$JAVA" -version 2>&1 | head -1)"
else
  echo "[check] java   : NOT FOUND — set JAVA_HOME to a JDK 21 (jPSXdec needs it)"
fi
JAR="${JPSXDEC_JAR:-}"
if [[ -z "$JAR" ]]; then JAR="$(ls "$HOME"/dev/jpsxdec/*/jpsxdec.jar "$HOME"/Applications/jpsxdec/*/jpsxdec.jar 2>/dev/null | head -1 || true)"; fi
if [[ -n "$JAR" && -e "$JAR" ]]; then echo "[check] jpsxdec: $JAR"; else echo "[check] jpsxdec: NOT FOUND — set JPSXDEC_JAR"; fi
if [[ -n "${AC1_BIN:-}" && -e "$AC1_BIN" ]]; then echo "[check] AC1_BIN: $AC1_BIN"; else echo "[check] AC1_BIN: NOT SET — open your SLUS-01323 .bin from the GUI instead"; fi
echo "[setup] ok. Run:  $HERE/run_ac1mod.sh"
