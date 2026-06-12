#!/usr/bin/env bash
# ghidra_overlay.sh — headless Ghidra decompile of AC1 code overlays (no GUI).
#
# Imports a carved overlay (disc_map/overlays/ovl*.bin) into a cached Ghidra
# project at base 0x8004ada0, auto-analyzes it ONCE, then decompiles any
# function address with full Ghidra quality + xrefs — the analysis the live
# Ghidra DB lacks for mission/mode overlays (see docs/MXT_LOADER.md).
#
# Usage:
#   tools/re/ghidra_overlay.sh <overlay.bin> <hexaddr> [hexaddr ...]
#   tools/re/ghidra_overlay.sh disc_map/overlays/ovl202_mission.bin 0x8008ab68
#
# First call on an overlay analyzes (~30s); later calls are fast (-process).
set -euo pipefail
HERE="$(cd "$(dirname "$0")/../.." && pwd)"
HL=/snap/ghidra/current/ghidra_12.0_PUBLIC/support/analyzeHeadless
BASE=0x8004ada0
PROJDIR=/tmp/ghidra_overlays

bin="$1"; shift
[ -f "$bin" ] || { echo "no such overlay bin: $bin" >&2; exit 1; }
name="$(basename "$bin" .bin)"
mkdir -p "$PROJDIR"

# import + analyze once (project/program cached under $PROJDIR)
if [ ! -e "$PROJDIR/$name.rep" ] && [ ! -e "$PROJDIR/$name.gpr" ]; then
  echo "[ghidra_overlay] importing+analyzing $name (one-time ~30s)..." >&2
  "$HL" "$PROJDIR" "$name" -import "$bin" \
     -processor "MIPS:LE:32:default" -loader BinaryLoader -loader-baseAddr "$BASE" \
     -scriptPath "$HERE/ghidra_scripts" >/tmp/${name}_import.log 2>&1 \
     || { echo "import failed; see /tmp/${name}_import.log" >&2; exit 1; }
fi

if [ "$#" -eq 0 ]; then echo "(imported; pass addresses to decompile)"; exit 0; fi

log="$(mktemp)"
"$HL" "$PROJDIR" "$name" -process "$(basename "$bin")" -noanalysis \
   -scriptPath "$HERE/ghidra_scripts" -postScript DecompAt.java "$@" >"$log" 2>&1 || true
# print everything between DECOMP markers (Ghidra only prefixes the 1st line of a
# multi-line println, so strip the prefix where present and drop the trailing tag)
awk '/DECOMP_BEGIN/{f=1} f{print} /DECOMP_END/{f=0}' "$log" \
  | sed -e 's/^INFO  DecompAt.java> //' -e 's/ (GhidraScript)  *$//'
rm -f "$log"
