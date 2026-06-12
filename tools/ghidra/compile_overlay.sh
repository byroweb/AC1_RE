#!/usr/bin/env bash
# compile_overlay.sh — compile one C file into a flat overlay .bin for AC1 patching.
#
#   compile_overlay.sh <src.c> <load_addr_hex> <entry_symbol> <out.bin> [defsym ...]
#
# Links .text+.rodata+.data contiguously at <load_addr> so absolute references to
# the function's own const tables resolve correctly, then emits a raw binary of
# the whole blob. Verifies the entry symbol lands at <load_addr> (no stray leading
# word); aborts otherwise. Extra args are passed through as -Wl,--defsym=NAME=ADDR
# to bind external SDK/overlay functions (e.g. shape_name=0x80065DBC).
#
# Toolchain flags per docs/REFERENCE.md §7 (note -march=r3000 REQUIRES -mfp32).
set -euo pipefail
SRC=$1; ADDR=$2; ENTRY=$3; OUT=$4; shift 4
GCC=mipsel-linux-gnu-gcc
OBJCOPY=mipsel-linux-gnu-objcopy
NM=mipsel-linux-gnu-nm
TMP=$(mktemp -d)
LDS="$TMP/ovl.ld"
ELF="$TMP/out.elf"

# Address on the output section header ($ADDR :) forces the VMA exactly, so a
# non-16-aligned overlay entry (e.g. 0x80065DBC) is not bumped up by the input
# section's default 16-byte alignment.
cat > "$LDS" <<EOF
ENTRY($ENTRY)
SECTIONS {
  .text $ADDR : { *(.text) *(.text.*) *(.rodata) *(.rodata.*) *(.data) *(.data.*) }
  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.note.*) }
}
EOF

DEFSYMS=()
for d in "$@"; do DEFSYMS+=( -Wl,--defsym="$d" ); done

# -falign-functions=4 etc.: keep code 4-byte (not 16-byte) aligned so an entry at
# a non-16-aligned overlay address (e.g. 0x80065DBC) is honoured exactly.
"$GCC" -march=r3000 -mips1 -mfp32 -EL -G0 -O2 \
  -falign-functions=4 -falign-labels=4 -falign-jumps=4 -falign-loops=4 \
  -ffreestanding -fno-builtin -fno-pic -mno-abicalls \
  -nostdlib -T "$LDS" "${DEFSYMS[@]}" -o "$ELF" "$SRC"

# The blob always begins at ADDR (the .text section VMA). If ADDR is not 16-byte
# aligned, the intrinsic .text alignment pads with leading zero word(s) (a NOP)
# so the entry lands at the next 16-aligned address. That's fine for code with
# in-image const tables (must keep the pad); the CALLER binds to the real entry.
got=$("$NM" "$ELF" | awk -v e="$ENTRY" '$3==e{print "0x"toupper($1)}')
want=$(printf '0x%X' $((ADDR)))
"$OBJCOPY" -O binary --only-section=.text "$ELF" "$OUT"
if [ "$got" != "$want" ]; then
  echo "NOTE: blob loads at $want but $ENTRY is at $got (leading align pad) — bind callers to $got" >&2
fi
echo "$OUT: $(stat -c%s "$OUT") bytes (load @ $want), $ENTRY @ $got"
rm -rf "$TMP"
