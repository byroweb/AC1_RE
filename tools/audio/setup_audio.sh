#!/usr/bin/env bash
# setup_audio.sh — one-time setup for the voice pipeline (Git Bash on Windows, or Linux).
#
#   tools/audio/setup_audio.sh            # create tools/audio/.venv, install deps, health check
#   tools/audio/setup_audio.sh --check    # health check only
#
# Installs faster-whisper (CTranslate2, CPU), torch CPU wheels + SpeechBrain for
# speaker embeddings, rapidfuzz, numpy/scipy/soundfile. Whisper/ECAPA model
# weights download on first use (~3 GB large-v3 into the Hugging Face cache,
# ~80 MB ECAPA into disc_map/extracted/AUDIO/_models).
#
# Java + jPSXdec are NOT installed here; see tools/README.md § jPSXdec.
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
  "$PY" -m pip install --quiet --index-url https://download.pytorch.org/whl/cpu --extra-index-url https://pypi.org/simple \
      -r "$HERE/requirements.txt"
fi

echo "[check] python : $("$PY" --version 2>&1)  ($PY)"
"$PY" - <<'EOF'
import importlib
for m in ("numpy", "scipy", "soundfile", "rapidfuzz", "faster_whisper", "ctranslate2", "torch", "torchaudio", "speechbrain"):
    try:
        mod = importlib.import_module(m)
        print(f"[check] {m:<14} {getattr(mod, '__version__', '?')}")
    except Exception as e:
        print(f"[check] {m:<14} MISSING ({e})")
EOF
JAVA="${JAVA_HOME:+$JAVA_HOME/bin/java}"; JAVA="${JAVA:-$(command -v java || true)}"
if [[ -n "$JAVA" && -e "$JAVA" || -n "$JAVA" && -e "$JAVA.exe" ]]; then
  echo "[check] java   : $("$JAVA" -version 2>&1 | head -1)"
else
  echo "[check] java   : NOT FOUND — set JAVA_HOME to a JDK 21 (jPSXdec needs it)"
fi
JAR="${JPSXDEC_JAR:-}"
if [[ -z "$JAR" ]]; then JAR="$(ls "$HOME"/dev/jpsxdec/*/jpsxdec.jar "$HOME"/Applications/jpsxdec/*/jpsxdec.jar 2>/dev/null | head -1 || true)"; fi
if [[ -n "$JAR" && -e "$JAR" ]]; then echo "[check] jpsxdec: $JAR"; else echo "[check] jpsxdec: NOT FOUND — set JPSXDEC_JAR (tools/README.md § jPSXdec)"; fi
if [[ -n "${AC1_BIN:-}" && -e "$AC1_BIN" ]]; then echo "[check] AC1_BIN: $AC1_BIN"; else echo "[check] AC1_BIN: NOT SET — point it at your SLUS-01323 v1.1 .bin"; fi
echo "[setup] ok. Run:  $PY $HERE/run_all.py"
