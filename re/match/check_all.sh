#!/usr/bin/env bash
# check_all.sh - re-verify every matched function in re/match/manifest.tsv with the oracle.
# Exit 0 only if every row prints MATCH. Usage: re/match/check_all.sh [-q]
#
# Manifest columns: vaddr, unit (201..204 = program overlay, exe, fdatNNN = mission
# overlay), source (relative to re/match), size, extra match_fn.py args.  The file
# may arrive with CRLF endings (core.autocrlf), so every field is stripped of \r.
# `python tools/loop/drive.py verify` is the same check in Python.
#
# Twin rows (tools/loop/twins.py replay): a fdatNNN row whose source lives in
# another unit's directory is a clone compiled from that shared C file with
# its own --base/--defsym; it is verified like any other row and counted
# separately.  Every MATCH in twin_replay.tsv must still have its manifest row.
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2                         # manifest paths (-I include, --syms ...) are repo-relative
PY=python3; command -v python3 >/dev/null 2>&1 || PY=python
fail=0; n=0; twins=0
while IFS=$'\t' read -r vaddr unit src size extra; do
  vaddr=${vaddr%$'\r'}; unit=${unit%$'\r'}; src=${src%$'\r'}; size=${size%$'\r'}; extra=${extra%$'\r'}
  case "$vaddr" in ''|'#'*) continue ;; esac
  n=$((n+1))
  case "$unit" in fdat*) [ "${src%%/*}" != "$unit" ] && twins=$((twins+1)) ;; esac
  flags=()
  case "$unit" in
    ''|exe) ;;                                   # exe addresses are unambiguous
    [0-9]*) flags=(--ovl "$unit") ;;             # program overlay 201..204
    fdat*)                                       # mission overlay: image + base from units.py
      img="$("$PY" "$ROOT/tools/loop/units.py" image "$unit")" || { printf 'FAIL  %s  ERROR: units.py image %s\n' "$vaddr" "$unit"; fail=$((fail+1)); continue; }
      flags=(--bin "$img")
      case " $extra " in *" --base "*) ;; *) flags+=(--base "$("$PY" "$ROOT/tools/loop/units.py" base "$unit")") ;; esac ;;
    *) printf 'FAIL  %s  ERROR: unknown unit %s\n' "$vaddr" "$unit"; fail=$((fail+1)); continue ;;
  esac
  [ -n "$size" ] && flags+=(--expect-size "$size")
  # shellcheck disable=SC2086
  line="$("$PY" "$ROOT/tools/match/match_fn.py" "$vaddr" "$HERE/$src" "${flags[@]}" $extra -q | tail -1)"
  case "$line" in MATCH*) printf 'ok    %s  %s\n' "$vaddr" "$line" ;;
                  *)      printf 'FAIL  %s  %s\n' "$vaddr" "$line"; fail=$((fail+1)) ;; esac
done < "$HERE/manifest.tsv"
REPORT="$HERE/twin_replay.tsv"
if [ -f "$REPORT" ]; then
  while IFS=$'\t' read -r _g tsrc _from tunit tva _sz tres _rest; do
    tres=${tres%$'\r'}
    case "$_g" in ''|'#'*) continue ;; esac
    [ "$tres" = MATCH ] || continue
    if ! tr -d '\r' < "$HERE/manifest.tsv" |
         awk -F'\t' -v u="$tunit" -v v="$tva" -v s="$tsrc" '$1==v && $2==u && $3==s {f=1} END {exit !f}'; then
      printf 'FAIL  %s  twin %s in twin_replay.tsv has no manifest row\n' "$tva" "$tunit"; fail=$((fail+1))
    fi
  done < "$REPORT"
fi
[ "$twins" -gt 0 ] && echo "($twins of the rows are replayed twins, see twin_replay.tsv)"
echo "$((n-fail))/$n matched"
[ "$fail" -eq 0 ]
