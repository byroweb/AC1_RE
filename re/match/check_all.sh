#!/usr/bin/env bash
# check_all.sh - re-verify every matched function in re/match/manifest.tsv with the oracle.
# Exit 0 only if every row prints MATCH. Usage: re/match/check_all.sh [-q]
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
fail=0; n=0
while IFS=$'\t' read -r vaddr ovl src size extra; do
  case "$vaddr" in ''|'#'*) continue ;; esac
  n=$((n+1))
  # shellcheck disable=SC2086
  line="$("$PY" "$ROOT/tools/match/match_fn.py" "$vaddr" "$HERE/$src" --ovl "$ovl" --expect-size "$size" $extra -q | tail -1)"
  case "$line" in MATCH*) printf 'ok    %s  %s\n' "$vaddr" "$line" ;;
                  *)      printf 'FAIL  %s  %s\n' "$vaddr" "$line"; fail=$((fail+1)) ;; esac
done < "$HERE/manifest.tsv"
echo "$((n-fail))/$n matched"
[ "$fail" -eq 0 ]
