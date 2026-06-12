#!/usr/bin/env bash
# overlay2c.sh — decompile a slice of AC1 MIPS code to C via objdump + m2c.
#
# The name/menu code lives in a runtime overlay that Ghidra's base-exe project
# doesn't cover, so I usually disassemble it live. This turns a raw byte range
# (from the overlay dump, or any binary) into readable C in one step.
#
# Usage:
#   tools/re/overlay2c.sh <hex_addr> <len_bytes> [binfile] [base_addr]
#     hex_addr  : runtime VA of the function start (e.g. 0x80082190)
#     len_bytes : number of bytes to decode (decimal or 0x..)
#     binfile   : source binary (default: overlay_80050000.bin)
#     base_addr : VA the binfile is mapped at (default: 0x80050000)
#
# Example: tools/re/overlay2c.sh 0x80082190 0x1b0
set -euo pipefail

HERE="$(cd "$(dirname "$0")/../.." && pwd)"
ADDR="${1:?need hex addr}"
LEN="${2:?need length}"
BIN="${3:-$HERE/overlay_80050000.bin}"
BASE="${4:-0x80050000}"

M2C="/home/byron/Applications/m2c"
OBJDUMP="mipsel-linux-gnu-objdump"

addr=$(( ADDR ))
len=$(( LEN ))
base=$(( BASE ))
off=$(( addr - base ))
[ "$off" -ge 0 ] || { echo "addr 0x$(printf %x $addr) is below base 0x$(printf %x $base)"; exit 1; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# 1. carve the byte slice
dd if="$BIN" of="$tmp/slice.bin" bs=1 skip="$off" count="$len" status=none

# 2. disassemble as little-endian MIPS-I, VMA-adjusted to the runtime address
"$OBJDUMP" -D -b binary -m mips:isa32 -EL \
  --adjust-vma="$addr" "$tmp/slice.bin" > "$tmp/dump.txt"

# 3. convert objdump -> m2c asm (labels for branch targets, $-prefixed regs)
python3 "$HERE/tools/re/objdump2m2c.py" "$addr" < "$tmp/dump.txt" > "$tmp/fn.s"

# 4. decompile
"$M2C/.venv/bin/python" "$M2C/m2c.py" -t mips-ido-c "$tmp/fn.s"
