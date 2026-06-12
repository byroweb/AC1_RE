#!/usr/bin/env python3
"""objdump2m2c.py — convert `mipsel objdump -D` output into m2c-ready GAS asm.

Reads objdump text on stdin, writes one function's asm on stdout:
  - prefixes registers with '$'
  - turns in-range branch targets into local labels (.L<addr>) and emits them
  - turns jal/j targets into func_<addr> (m2c renders these as calls)
Usage: objdump -D ... | objdump2m2c.py <start_hex> [fn_name]
"""
import re
import sys

start = int(sys.argv[1], 16)
fn_name = sys.argv[2] if len(sys.argv) > 2 else f"fn_{start:x}"

REGS = (r"zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|k[01]|gp|sp|fp|ra|"
        r"f[0-9]|f[12][0-9]|f3[01]")
reg_re = re.compile(r"\b(" + REGS + r")\b")
line_re = re.compile(r"^\s*([0-9a-f]+):\t[0-9a-f ]+\t(\S+)\s*(.*)$")
# instructions whose final operand is a code address
BRANCH = {"b", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez",
          "bgezal", "bltzal", "beql", "bnel", "j", "jal", "bc1t", "bc1f"}

insns = []          # (addr, mnem, ops)
for raw in sys.stdin:
    m = line_re.match(raw.rstrip("\n"))
    if not m:
        continue
    addr = int(m.group(1), 16)
    insns.append((addr, m.group(2), m.group(3).strip()))

lo = insns[0][0] if insns else start
hi = insns[-1][0] if insns else start
defined = {a for a, _, _ in insns}
targets = set()         # branch targets -> local labels (in or out of slice)
for addr, mnem, ops in insns:
    if mnem in BRANCH and ops and mnem not in ("j", "jal"):
        last = ops.split(",")[-1].strip()
        mt = re.fullmatch(r"0x([0-9a-f]+)|([0-9a-f]+)", last)
        if mt:
            targets.add(int(last, 16))

def fix_ops(mnem, ops):
    if not ops:
        return ""
    parts = [p.strip() for p in ops.split(",")]
    last = parts[-1]
    mt = re.fullmatch(r"0x([0-9a-f]+)|([0-9a-f]+)", last)
    if mnem in BRANCH and mt:
        t = int(last, 16)
        if mnem in ("j", "jal"):
            parts[-1] = f"func_{t:x}"
        elif t in targets:
            parts[-1] = f".L{t:x}"
    ops = ", ".join(parts)
    return reg_re.sub(r"$\1", ops)

out = [f"glabel {fn_name}"]
for addr, mnem, ops in insns:
    if addr in targets:
        out.append(f".L{addr:x}:")
    out.append(f"    {mnem} {fix_ops(mnem, ops)}".rstrip())
# Branch targets past the decoded slice (len too small / forward branch out of
# the function): emit trailing stub labels so m2c still parses the in-range body.
missing = sorted(t for t in targets if t not in defined)
if missing:
    sys.stderr.write(
        "objdump2m2c: WARNING %d branch target(s) past the slice "
        "(len too small?): %s\n" % (len(missing), ", ".join(f"0x{m:x}" for m in missing)))
    for t in missing:
        out.append(f".L{t:x}:")
    out.append("    jr $ra")
    out.append("    nop")
print("\n".join(out))
