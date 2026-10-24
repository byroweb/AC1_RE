#!/usr/bin/env python3
"""structdiff.py - where and how a near miss's *shape* differs from retail, in C terms.

    structdiff.py UNIT VADDR [CFILE] [--context 4]

A structural miss (different length, or the branches sit elsewhere) needs a different way
of writing the C, not a search.  Most of an agent's tries go on finding *which* rewrite.
This tool compiles the C exactly like the oracle, keeps the object, and reads its line
table (cc1 -gcoff emits .loc; GNU as turns it into .debug_line), so every one of our
instructions has a C line.  Then it compares the two functions' shapes with register
names ignored and reports named findings:

  first divergence  the first region where our instructions and retail's part ways, as
                    C lines, with both sides side by side
  loops             per loop, in order: tested at the bottom (one conditional backward
                    branch: do-while / GCC's rotated while) or at the top (a conditional
                    exit plus an unconditional jump back)
  switch            a jump table (jr through a table) on one side, a compare chain on
                    the other: this cc1 makes a table when there are >= 5 cases spanning
                    <= 10x the count
  returns / tails   the number of return paths, and blocks that occur twice on one side
                    and once on the other: cross-jumping merged (or kept) a shared tail
  calls             callees called more or less often
  branches          the same compare with the opposite sense: then/else arms swapped
  frame             the stack frame size (spill slots, local arrays)

Each finding names the C lines it comes from and the idiom that usually fixes it (the
GCC 2.7.2 levers the matching agents found; .claude/skills/match-function/SKILL.md).
doctor.py runs it for the size / structure classes.
"""
import argparse
import bisect
import collections
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
import match_fn  # noqa: E402
import permute  # noqa: E402
import regtie  # noqa: E402

COND_BRANCH = re.compile(r"^(beq|bne|beqz|bnez|bltz|bgez|blez|bgtz|bltzal|bgezal)$")
UNCOND = re.compile(r"^(b|j)$")
HEX = re.compile(r"^0x[0-9a-f]+$")
NEG = {"beq": "bne", "bne": "beq", "beqz": "bnez", "bnez": "beqz", "bltz": "bgez", "bgez": "bltz",
       "blez": "bgtz", "bgtz": "blez"}


# ---------------------------------------------------------------------------
# build with the line table
# ---------------------------------------------------------------------------
def readelf_path(tools):
    d = os.path.dirname(tools["as"])
    for n in ("mipsel-none-elf-readelf.exe", "mipsel-none-elf-readelf", "mipsel-linux-gnu-readelf",
              "readelf"):
        p = os.path.join(d, n)
        if os.path.isfile(p):
            return p
    return shutil.which("readelf")


def build(meta, syms, text, inc):
    """-> dict(bytes, rows, lines: [(offset, fn.i line)], fni: fn.i text) or None."""
    tools = meta["tools"]
    work = tempfile.mkdtemp(prefix="structdiff_")
    try:
        c = os.path.join(work, "fn.c")
        with open(c, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        permute.preprocess(tools, c, os.path.join(work, "fn.i"), [inc])
        obj = permute.build_object(meta, os.path.join(work, "fn.i"), work, tools)
        blob, _unres = permute.resolve_object(obj, None, meta, syms)
        with open(obj, "rb") as fh:
            elf = match_fn.Elf(fh.read())
        _n, start, size = match_fn.pick_function(elf, meta["fn"])
        lines = []
        rp = readelf_path(tools)
        if rp:
            out = subprocess.run([rp, "--debug-dump=decodedline", obj], capture_output=True, text=True,
                                 errors="replace").stdout
            for m in re.finditer(r"^\S+\s+(\d+)\s+(0x[0-9a-f]+|\d+)\b", out, re.M):
                off = int(m.group(2), 0) - start
                if 0 <= off < size:
                    lines.append((off, int(m.group(1))))
        lines.sort()
        with open(os.path.join(work, "fn.i"), encoding="utf-8", errors="replace") as fh:
            fni = fh.read()
        return {"bytes": blob, "lines": lines, "fni": fni}
    except (RuntimeError, match_fn.ToolError, OSError, ValueError):
        return None
    finally:
        shutil.rmtree(work, ignore_errors=True)


class LineMap:
    """offset -> fn.i line -> the C file's line (by the text of the line)."""

    def __init__(self, lines, fni, ctext):
        self.offs = [o for o, _l in lines]
        self.ilines = [l for _o, l in lines]
        self.fni = fni.split("\n")
        self.c = ctext.split("\n")
        self.cache = {}

    def iline(self, off):
        k = bisect.bisect_right(self.offs, off) - 1
        return self.ilines[k] if k >= 0 else None

    def cline(self, iline):
        """1-based line of the C file whose text equals fn.i's line (whitespace-insensitive)."""
        if iline is None or iline < 1 or iline > len(self.fni):
            return None
        if iline in self.cache:
            return self.cache[iline]
        key = " ".join(self.fni[iline - 1].split())
        hit = None
        if key:
            cands = [i + 1 for i, t in enumerate(self.c) if " ".join(t.split()) == key]
            if len(cands) == 1:
                hit = cands[0]
            elif cands:
                # the same text more than once: the candidate nearest in relative position
                rel = iline / max(1, len(self.fni))
                hit = min(cands, key=lambda c: abs(c / max(1, len(self.c)) - rel))
        self.cache[iline] = hit
        return hit

    def lines_of(self, offs):
        out = sorted({self.cline(self.iline(o)) for o in offs} - {None})
        return out

    def text(self, cline):
        return self.c[cline - 1].strip() if cline and cline <= len(self.c) else ""


# ---------------------------------------------------------------------------
# shape
# ---------------------------------------------------------------------------
def parse(rows, base):
    """[(op, toks, target or None)] with branch targets as offsets from the function start."""
    out = []
    for r in rows:
        op, toks = regtie.split_ops(r)
        tgt = None
        if (COND_BRANCH.match(op) or UNCOND.match(op)) and toks and HEX.match(toks[-1]):
            tgt = int(toks[-1], 16) - base
        out.append((op, toks, tgt))
    return out


def shape_token(ins):
    """An instruction with register names and branch targets abstracted."""
    op, toks, tgt = ins
    t = []
    for x in toks:
        if regtie.REG_TOKEN.match(x):
            t.append("R")
        elif tgt is not None and HEX.match(x):
            t.append("@")
        elif HEX.match(x) and int(x, 16) >= 0x80000000:
            t.append("A")                      # an absolute address (call target)
        else:
            t.append(x)
    return (op,) + tuple(t)


def loops(ins):
    """[(head offset, tail offset, kind)] per backward branch, in order of the tail."""
    out = []
    for i, (op, _t, tgt) in enumerate(ins):
        off = 4 * i
        if tgt is not None and tgt <= off:
            out.append((tgt, off, "bottom-test" if COND_BRANCH.match(op) else "top-test"))
    return out


def blocks(ins):
    """{start: tuple(shape tokens)} basic blocks (leaders: 0, branch targets, after a delay slot)."""
    leaders = {0}
    for i, (op, _t, tgt) in enumerate(ins):
        if tgt is not None:
            leaders.add(tgt)
            leaders.add(4 * (i + 2))
        elif op in ("jr", "jalr"):
            leaders.add(4 * (i + 2))
    leaders = sorted(x for x in leaders if 0 <= x < 4 * len(ins))
    out = {}
    for k, s in enumerate(leaders):
        e = leaders[k + 1] if k + 1 < len(leaders) else 4 * len(ins)
        out[s] = tuple(shape_token(x) for x in ins[s // 4:e // 4])
    return out


def frame(ins):
    for op, toks, _t in ins[:4]:
        if op == "addiu" and toks[:2] == ["sp", "sp"] and toks[2].startswith("-"):
            return -int(toks[2], 0)
    return 0


def calls(ins):
    return collections.Counter(t[-1] for op, t, _ in ins if op == "jal" and t)


# ---------------------------------------------------------------------------
def findings(ours_rows, ret_rows, base, lm, context=4):
    o, r = parse(ours_rows, base), parse(ret_rows, base)
    out = []

    def where(offs):
        ls = lm.lines_of(offs) if lm else []
        if not ls:
            return ""
        return "C line%s %s" % ("s" if len(ls) > 1 else "", ", ".join(map(str, ls[:6])))

    # first divergence, on the abstracted shape
    so, sr = [shape_token(x) for x in o], [shape_token(x) for x in r]
    sm = difflib.SequenceMatcher(a=so, b=sr, autojunk=False)
    ops = [x for x in sm.get_opcodes() if x[0] != "equal"]
    if ops:
        tag, i1, i2, j1, j2 = ops[0]
        span = range(max(0, i1 - 1), min(len(o), max(i2, i1 + 1) + 1))
        side = []
        for k in range(max(0, min(i1, j1) - context), max(i2, j2) + context):
            a = ours_rows[k] if k < len(ours_rows) and i1 - context <= k < i2 + context else ""
            b = ret_rows[k - i1 + j1] if 0 <= k - i1 + j1 < len(ret_rows) and j1 - context <= k - i1 + j1 < j2 + context \
                else ""
            if a or b:
                mark = "!" if i1 <= k < max(i2, i1 + 1) else " "
                side.append("      %s %-34s | %s" % (mark, a[:34], b))
        out.append(("divergence", "first shape difference at our +0x%x / retail +0x%x (%s, %d vs %d instructions) %s"
                    % (4 * i1, 4 * j1, tag, i2 - i1, j2 - j1, where([4 * k for k in span])), side))
        out.append(("regions", "%d differing regions in all (ours %d / retail %d instructions in them)"
                    % (len(ops), sum(x[2] - x[1] for x in ops), sum(x[4] - x[3] for x in ops)), []))

    # loops
    lo, lr = loops(o), loops(r)
    if len(lo) != len(lr):
        out.append(("loops", "ours has %d loops, retail %d" % (len(lo), len(lr)), []))
    for k, (a, b) in enumerate(zip(lo, lr)):
        if a[2] != b[2]:
            fix = ("write it as do { ... } while (cond); (guarded by if (cond) when it may run 0 times)"
                   if b[2] == "bottom-test" else "write it as while (cond) { ... } / for (;;) with a break")
            out.append(("loop", "loop %d (%s): ours %s, retail %s: %s" % (k + 1, where(range(a[0], a[1] + 4, 4)),
                                                                         a[2], b[2], fix), []))

    # switch lowering
    jo = sum(1 for op, _t, _g in o if op == "jr" and _t and _t[0] != "ra")
    jr = sum(1 for op, _t, _g in r if op == "jr" and _t and _t[0] != "ra")
    if jo != jr:
        msg = ("retail uses %d jump table(s), ours %d: " % (jr, jo) +
               ("write the cases as an if/goto chain laid out like GCC's case tree, or drop a case "
                "(>= 5 cases spanning <= 10x their count always make a table here)" if jo > jr else
                "use a switch with >= 5 cases spanning <= 10x their count (add the missing cases/defaults)"))
        out.append(("switch", msg, []))

    # returns and duplicated tails
    ro = sum(1 for op, t, _g in o if op == "jr" and t == ["ra"])
    rr = sum(1 for op, t, _g in r if op == "jr" and t == ["ra"])
    if ro != rr:
        out.append(("returns", "return paths: ours %d, retail %d (a shared epilogue reached by a jump vs a "
                               "duplicated one; per-path `return` vs one exit)" % (ro, rr), []))
    bo, br = blocks(o), blocks(r)
    co, cr = collections.Counter(bo.values()), collections.Counter(br.values())
    dup = []
    for blk, n in cr.items():
        m = co.get(blk, 0)
        if len(blk) >= 3 and n != m and min(n, m) >= 1:
            starts = [s for s, b in bo.items() if b == blk]
            dup.append((n, co.get(blk, 0), starts))
    for blk, m in co.items():
        n = cr.get(blk, 0)
        if len(blk) >= 3 and m > n >= 1:
            dup.append((n, m, [s for s, b in bo.items() if b == blk]))
    seen_dup = set()
    dup = [d for d in dup if not (tuple(d[2]) in seen_dup or seen_dup.add(tuple(d[2])))]
    for n_r, n_o, starts in sorted(dup, key=lambda x: -abs(x[0] - x[1]))[:3]:
        out.append(("tail", "a %s block occurs %d times in retail, %d in ours %s: %s"
                    % ("repeated" if n_r > 1 else "single", n_r, n_o, where(starts),
                       "cross-jumping merged it in ours: give each arm its own copy (separate calls/stores per "
                       "arm, a per-path return)" if n_r > n_o else
                       "retail merged it: jump to one shared copy (goto a label after the first arm)"), []))

    # calls
    ca, cb = calls(o), calls(r)
    for tgt in sorted(set(ca) | set(cb)):
        if ca.get(tgt, 0) != cb.get(tgt, 0):
            starts = [4 * i for i, (op, t, _g) in enumerate(o) if op == "jal" and t and t[-1] == tgt]
            out.append(("call", "calls to %s: ours %d, retail %d %s" % (tgt, ca.get(tgt, 0), cb.get(tgt, 0),
                                                                    where(starts)), []))

    # branch sense on aligned compares
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != "replace" or i2 - i1 != j2 - j1:
            continue
        for k in range(i2 - i1):
            a, b = o[i1 + k], r[j1 + k]
            if NEG.get(a[0]) == b[0] and len(a[1]) == len(b[1]):
                out.append(("branch", "%s here is %s in retail %s: the then/else arms are laid out the other "
                                      "way round (invert the condition and swap the arms, or move the "
                                      "else body first)" % (a[0], b[0], where([4 * (i1 + k)])), []))
                break

    # jumps that land elsewhere: same jump, different target block (delay-slot stealing, a
    # label inside a shared block vs after it)
    jumps = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != "equal":
            continue
        for k in range(i2 - i1):
            a, b = o[i1 + k], r[j1 + k]
            if a[2] is None or b[2] is None or jumps >= 3:
                continue
            # compare targets through the alignment: where would our target sit in retail?
            ta = a[2] // 4
            mapped = None
            for t2, x1, x2, y1, y2 in sm.get_opcodes():
                if t2 == "equal" and x1 <= ta < x2:
                    mapped = y1 + (ta - x1)
            if mapped is not None and mapped != b[2] // 4:
                jumps += 1
                d = 4 * (b[2] // 4 - mapped)
                out.append(("jump", "%s at our +0x%x lands %+d bytes from where retail's lands %s: %s"
                            % (a[0], 4 * (i1 + k), d, where([4 * (i1 + k)]),
                               "retail jumps into the middle of the shared block (a label before its last "
                               "statement / the delay slot stolen from the target)" if d < 0 else
                               "retail jumps past the block start (its first insn moved into the delay slot)"), []))

    # frame
    fo, fr = frame(o), frame(r)
    if fo != fr:
        out.append(("frame", "stack frame: ours %d, retail %d bytes (%+d): %s" % (
            fo, fr, fr - fo, "retail has more spill slots or a bigger local (an unused SVECTOR/pad local, "
            "or one fewer value kept in a register)" if fr > fo else "retail keeps fewer stack slots (drop a local, "
            "narrow a type, keep a value in a register)"), []))
    return out


def run(unit, vaddr, cfile, context=4):
    work = tempfile.mkdtemp(prefix="structdiff_run_")
    try:
        meta, syms, retail, text = regtie.load(unit, vaddr, cfile, work)
        res = build(meta, syms, text, os.path.dirname(os.path.abspath(cfile)))
        if res is None:
            return None, "does not build", []
        lm = LineMap(res["lines"], res["fni"], text) if res["lines"] else None
        ours_rows = regtie.disasm_rows(meta, res["bytes"])
        ret_rows = regtie.disasm_rows(meta, retail)
        words = permute.word_score(res["bytes"], retail)
        head = "%s 0x%08X: %d differing words, ours %d / retail %d bytes%s" % (
            unit, vaddr, words, len(res["bytes"]), len(retail), "" if lm else " (no line table)")
        return lm, head, findings(ours_rows, ret_rows, vaddr, lm, context)
    finally:
        shutil.rmtree(work, ignore_errors=True)


def render(lm, head, fs):
    lines = [head]
    for kind, msg, extra in fs:
        lines.append("  [%s] %s" % (kind, msg))
        lines += extra
    if lm and fs:
        cl = re.findall(r"C lines? ([\d, ]+)", fs[0][1])
        if cl:
            for n in [int(x) for x in cl[0].split(",") if x.strip()][:4]:
                lines.append("      line %d: %s" % (n, lm.text(n)[:100]))
    return "\n".join(lines)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("unit")
    ap.add_argument("vaddr")
    ap.add_argument("cfile", nargs="?")
    ap.add_argument("--context", type=int, default=4)
    args = ap.parse_args(argv)
    vaddr = int(args.vaddr, 16)
    cfile = args.cfile or os.path.join(REPO, "re", "match", args.unit, "func_%08X.c" % vaddr)
    lm, head, fs = run(args.unit, vaddr, cfile, args.context)
    print(render(lm, head, fs))
    return 0


if __name__ == "__main__":
    sys.exit(main())
