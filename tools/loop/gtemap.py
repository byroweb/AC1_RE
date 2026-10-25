#!/usr/bin/env python3
"""gtemap.py - name the include/ac1/gte.h macro behind each GTE run in a retail function.

    python tools/loop/gtemap.py <unit> <vaddr> [--size N]

m2c does not know the GTE (coprocessor 2) instructions: it prints each `lwc2`, `ctc2`,
`mtc2` or `cop2` command as `M2C_ERROR(/* unknown instruction ... */)`.  This reads the
retail words instead and matches every macro of include/ac1/gte.h against them (an
`asm volatile` block is emitted as one contiguous run), longest macro first, so each
GTE run prints as the macro call to write, with the register that carries each
argument:

    8008cdec  gte_ldv0($19)            lwc2 $0,0($19) ; lwc2 $1,4($19)
    8008cdfc  gte_rtv0tr()             nop ; nop ; c2 0x480012

A GTE instruction no macro covers prints as `??`: write a local macro for it (the
retail words are the template) and propose it for gte.h.
"""
import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import units  # noqa: E402

GTE_H = os.path.join(REPO, "include", "ac1", "gte.h")
MACRO = re.compile(r"^#define (gte_\w+)\(([^)]*)\) __asm__ volatile \(((?:[^\n]*\\\n)*[^\n]*)", re.M)
GTE_OPS = re.compile(r"^(lwc2|swc2|mtc2|mfc2|ctc2|cfc2|c2)\b")
ARG = r"(\$\d+)"


def norm(ins):
    """`lw $12, 0(%0)` / objdump's `lw\t$12,0($19)` -> `lw $12,0($19)`."""
    ins = ins.strip().replace("\t", " ")
    ins = re.sub(r"\s*,\s*", ",", ins)
    if not ins.startswith("c2 "):          # objdump prints shift amounts in hex
        ins = re.sub(r"\b0x([0-9a-f]+)\b", lambda m: str(int(m.group(1), 16)), ins)
    return re.sub(r"\s+", " ", ins)


def template(ins):
    """One gte.h instruction -> regex over normalised objdump text (%N captures a reg)."""
    m = re.match(r"\.word 0x([0-9A-Fa-f]{8})$", ins)
    if m:
        return re.escape("c2 0x%x" % (int(m.group(1), 16) & 0x1FFFFFF))
    parts = re.split(r"(%\d)", norm(ins))
    return "".join("(?P<a%s>\\$\\d+)" % p[1] if re.match(r"%\d$", p) else re.escape(p)
                   for p in parts)


def macros(path=GTE_H):
    """[(name, params, [regex per instruction])], longest first; names joined by | when
    two macros emit the same instructions."""
    text = open(path, encoding="utf-8").read().replace("\r\n", "\n")
    out = []
    for m in MACRO.finditer(text):
        body = m.group(3).replace("\\\n", " ")
        tmpl = "".join(re.findall(r'"((?:[^"\\]|\\.)*)"', body.split(":")[0]))
        ins = [i for i in re.split(r"\\n\\t|\\n|;", tmpl) if i.strip()]
        params = [p.strip() for p in m.group(2).split(",") if p.strip()]
        regs = [template(i) for i in ins]
        same = next((k for k, o in enumerate(out) if o[2] == regs and len(o[1]) == len(params)), None)
        if same is None:
            out.append((m.group(1), params, regs))
        else:                               # one cop2 word, two PsyQ names (gte_rt / gte_rtv0tr)
            out[same] = (out[same][0] + "|" + m.group(1), params, regs)
    out.sort(key=lambda t: -len(t[2]))
    return out


def listing(unit, vaddr, size):
    u = units.resolve(unit, need_image=True)
    cmd = [units.find_objdump(), "-D", "-z", "-b", "binary", "-m", "mips:3000", "-EL",
           "-M", "gpr-names=numeric", "--adjust-vma=0x%X" % u.base,
           "--start-address=0x%X" % vaddr, "--stop-address=0x%X" % (vaddr + size), u.image_path]
    res = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
    if res.returncode:
        sys.exit("gtemap: objdump failed: " + res.stderr.strip())
    rows = []
    for ln in res.stdout.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(.*)$", ln)
        if m:
            rows.append((int(m.group(1), 16), norm(m.group(2))))
    return rows


def scan(rows, table):
    """[(addr, call text, instruction texts)] for every GTE run, '??' where nothing fits."""
    out, i = [], 0
    while i < len(rows):
        hit = None
        for name, params, regs in table:
            if i + len(regs) > len(rows):
                continue
            args = {}
            for k, rx in enumerate(regs):
                m = re.fullmatch(rx, rows[i + k][1])
                if not m or any(args.setdefault(a, v) != v for a, v in m.groupdict().items()):
                    break
                args.update(m.groupdict())
            else:
                if any(GTE_OPS.match(r[1]) for r in rows[i:i + len(regs)]):
                    a = ", ".join(args.get("a%d" % n, "?") for n in range(len(params)))
                    call = " | ".join("%s(%s)" % (n, a) for n in name.split("|"))
                    hit = (rows[i][0], call, [r[1] for r in rows[i:i + len(regs)]])
                    i += len(regs)
                    break
        if hit:
            out.append(hit)
            continue
        if GTE_OPS.match(rows[i][1]):
            out.append((rows[i][0], "??", [rows[i][1]]))
        i += 1
    return out


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("unit")
    ap.add_argument("vaddr")
    ap.add_argument("--size", type=lambda s: int(s, 0), help="bytes (default: functions.tsv)")
    a = ap.parse_args(argv)
    vaddr = int(a.vaddr, 16)
    size = a.size
    if size is None:
        with open(os.path.join(HERE, "functions.tsv"), encoding="utf-8") as fh:
            for ln in fh:
                f = ln.split("\t")
                if len(f) > 2 and f[0] == a.unit and f[1].lower() == "0x%08x" % vaddr:
                    size = int(f[2])
        if size is None:
            sys.exit("gtemap: %s 0x%08X not in functions.tsv; pass --size" % (a.unit, vaddr))
    hits = scan(listing(a.unit, vaddr, size), macros())
    for addr, call, ins in hits:
        print("%08x  %-26s %s" % (addr, call, " ; ".join(ins)))
    unknown = sum(1 for h in hits if h[1] == "??")
    print("%d GTE run(s), %d not covered by gte.h" % (len(hits), unknown))
    return 1 if unknown else 0


if __name__ == "__main__":
    sys.exit(main())
