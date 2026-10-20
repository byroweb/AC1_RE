#!/usr/bin/env bash
# AC1mod launcher — runs the GUI from tools/ac1mod/.venv (create it with setup.sh).
cd "$(dirname "$(readlink -f "$0")")"
if [[ -x .venv/Scripts/python.exe ]]; then PY=.venv/Scripts/python.exe; else PY=.venv/bin/python; fi
exec "$PY" main.py "$@"
