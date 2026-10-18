#!/usr/bin/env bash
# Run the match_fn.py tests (pytest if installed, else a plain shell check of the two sources).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
if "$PY" -c "import pytest" 2>/dev/null; then
  exec "$PY" -m pytest -q "$HERE"
fi
echo "pytest not installed (pip install pytest); running the two basic checks only"
M="$HERE/../match_fn.py"
"$PY" "$M" 0x8004F7E8 "$HERE/func_8004F7E8.c" -q | tail -1 | grep -qx "MATCH func_8004F7E8 40" || { echo "FAIL: good source did not MATCH"; exit 1; }
"$PY" "$M" 0x8004F7E8 "$HERE/func_8004F7E8_perturbed.c" -q | tail -1 | grep -qx "MISMATCH func_8004F7E8 0x4" || { echo "FAIL: perturbed source did not MISMATCH at 0x4"; exit 1; }
echo "OK: MATCH on func_8004F7E8.c, MISMATCH 0x4 on func_8004F7E8_perturbed.c"
