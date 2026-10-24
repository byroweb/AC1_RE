#!/usr/bin/env python3
"""Near-copies: open functions with the same instruction shape as another function.

twins.py groups functions whose bodies are identical once relocation slots are masked. It
misses copies that differ in a few immediates, usually a struct field offset: the same
renderer reads its globals block at +0xB20 in ovl202 and at +0x670 in ovl203/204. Those
copies end up in separate groups, so a matched one is never replayed onto the others, and
two agents can work the same body twice.

  python tools/loop/nearport.py scan [--min 0.97]
      Families of open functions that share a shape (opcode/funct sequence) with each
      other or with a matched function. Give each family to one agent, and port the rest.

  python tools/loop/nearport.py port OUTDIR [--max-edits 40] [UNIT:VADDR ...]
      For each open function whose shape equals a matched one, copy the matched C, move
      its symbols the way twins.py replay does, then try single-literal edits that turn
      the matched function's differing immediates into the copy's (a pad size, an
      offset). Clean MATCHes are left in OUTDIR/<unit>/func_<ADDR>.c. Each prints a
      `unit:0xVADDR@NAME=ADDR...` item for promotion. Compiles run one at a time.
"""
import argparse
import difflib
import os
import re
import subprocess
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import progress  # noqa: E402
import twins  # noqa: E402

FUNCTIONS = os.path.join(HERE, "functions.tsv")
MANIFEST = os.path.join(REPO, "re", "match", "manifest.tsv")
SKIP_UNITS = {"fdat168"}
HAND_ASM = {("ovl201", 0x800B55E4), ("ovl201", 0x800B5618)}
NAME = re.compile(r"\b(D|func)_([0-9A-Fa-f]{8})\b")
LITERAL = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)(?![\w.])")


def shape(words):
    """Opcode (plus funct / regimm rt) per word: the instruction sequence without operands."""
    out = []
    for w in words:
        op = w >> 26
        out.append((op, w & 0x3F) if op == 0 else (op, (w >> 16) & 0x1F) if op == 1 else (op,))
    return tuple(out)


def inventory():
    """(open [(unit, va, size)], matched [(unit, va, size)])"""
    funcs = progress.load_functions(FUNCTIONS)
    for u in SKIP_UNITS:
        funcs.pop(u, None)
    matched = progress.load_matched(MANIFEST)
    opn, done = [], []
    for u, fs in funcs.items():
        for va, sz in fs.items():
            if (u, va) not in HAND_ASM:
                (done if (u, va) in matched else opn).append((u, va, sz))
    return opn, done


def manifest_rows():
    rows = {}
    for line in open(MANIFEST, encoding="utf-8"):
        p = line.rstrip("\r\n").split("\t")
        if len(p) >= 5 and p[0].startswith("0x"):
            try:
                rows[(twins.units.norm(p[1] or "exe"), int(p[0], 16))] = p
            except twins.units.UnitError:
                pass
    return rows


def words_of(u, va, sz):
    return twins.func_words(u, va, sz)


# ---------------------------------------------------------------------------
# scan
# ---------------------------------------------------------------------------
def families(min_ratio=0.97):
    """[(members [(unit, vaddr)], best)] for every open function. A family links open
    functions in one twin group or with shapes at least min_ratio alike. best is
    (ratio, unit, vaddr, member unit, member vaddr) of the closest matched function,
    or None. Families are sorted largest first."""
    opn, done = inventory()
    grp = {}
    for path in (twins.TWINS, twins.TWINS_PROGRAMS):
        for k, g in twins.read_twins(path)[0].items():
            grp[k] = g
    shp = {(u, va): shape(words_of(u, va, sz)) for u, va, sz in opn}
    # union-find over open functions linked by similar shape
    parent = {k: k for k in shp}

    def find(k):
        while parent[k] != k:
            parent[k] = parent[parent[k]]
            k = parent[k]
        return k
    keys = sorted(shp)
    for i, a in enumerate(keys):
        for b in keys[i + 1:]:
            sa, sb = shp[a], shp[b]
            if min(len(sa), len(sb)) / max(len(sa), len(sb)) < min_ratio:
                continue
            if (grp.get(a) and grp.get(a) == grp.get(b)) or \
                    difflib.SequenceMatcher(None, sa, sb, autojunk=False).ratio() >= min_ratio:
                parent[find(a)] = find(b)
    fam = defaultdict(list)
    for k in keys:
        fam[find(k)].append(k)
    # the closest matched function for each family (port source)
    done_shapes = defaultdict(list)
    for u, va, sz in done:
        done_shapes[sz // 16].append((u, va, sz))
    rows = []
    for members in fam.values():
        best = None
        for u, va in members:
            s = shp[(u, va)]
            n = len(s)
            for b in range(int(n * min_ratio * 4) // 16, int(n / min_ratio * 4) // 16 + 1):
                for mu, mva, msz in done_shapes.get(b, ()):
                    if min(n, msz // 4) / max(n, msz // 4) < min_ratio:
                        continue
                    r = difflib.SequenceMatcher(None, s, shape(words_of(mu, mva, msz)), autojunk=False).ratio()
                    if r >= min_ratio and (best is None or r > best[0]):
                        best = (r, mu, mva, u, va)
        rows.append((members, best))
    rows.sort(key=lambda r: (-len(r[0]), r[0][0]))
    return rows


def cmd_scan(args):
    rows = families(args.min)
    multi = [r for r in rows if len(r[0]) > 1 or r[1]]
    print("%d open functions, %d families; %d families have a twin-group split or a matched near-copy"
          % (sum(len(r[0]) for r in rows), len(rows), len(multi)))
    for members, best in multi:
        src = "port from %s 0x%08X (%.3f)" % (best[1], best[2], best[0]) if best else "no matched copy"
        print("  %-60s %s" % (" ".join("%s:0x%08X" % m for m in members), src))
    return 0


# ---------------------------------------------------------------------------
# port
# ---------------------------------------------------------------------------
def imm_pairs(rw, mw):
    """Unique (rep_imm, member_imm) of words that differ only in the low 16 bits."""
    out = []
    for a, b in zip(rw, mw):
        if a != b and (a ^ b) & 0xFFFF0000 == 0 and (a, b) not in out:
            out.append((a & 0xFFFF, b & 0xFFFF))
    return list(dict.fromkeys(out))


def literal_edits(text, rep_imm, mem_imm):
    """Edits L -> L + (mem - rep) for literals at or a little below rep_imm (a field offset,
    or the pad before the field): each as a list of spans, every use of one value first,
    then single uses. Comments are left alone."""
    d = twins.s16(mem_imm) - twins.s16(rep_imm)
    a = twins.s16(rep_imm)
    code = re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: " " * len(m.group(0)), text, flags=re.S)
    out = []
    for m in LITERAL.finditer(code):
        s = m.group(1)
        v = int(s, 0) if not (len(s) > 1 and s[0] == "0" and s[1] not in "xX") else int(s, 8)
        for base in (a, -a):
            if base - 0x200 <= v <= base and v + d >= 0:
                nv = v + d if base == a else v - d
                ns = ("0x%X" % nv) if s[:2].lower() == "0x" else str(nv)
                out.append([(m.start(1), m.end(1), ns)])
    # the same value written in several places (a screen limit tested twice) moves together
    byval = defaultdict(list)
    for e in out:
        byval[(int(code[e[0][0]:e[0][1]], 0) if code[e[0][0]:e[0][1]][:2].lower() == "0x"
               else int(code[e[0][0]:e[0][1]]), e[0][2])].append(e[0])
    return [spans for spans in byval.values() if len(spans) > 1] + out


def try_c(out, unit, va, text, defsyms):
    path = os.path.join(out, unit, "func_%08X.c" % va)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)
    argv = [sys.executable, os.path.join(HERE, "drive.py"), "try", unit, "0x%08X" % va, "--re-dir", out]
    for n, a in defsyms:
        argv += ["--defsym", "%s=0x%08X" % (n, a)]
    r = subprocess.run(argv, cwd=REPO, capture_output=True, text=True, errors="replace")
    res = [ln for ln in r.stdout.splitlines() if ln.startswith("result:")]
    return r.returncode == 0, (res[0] if res else r.stdout.strip()[-200:])


def port_one(out, unit, va, sz, src_key, rows, tsyms_cache, max_edits):
    row = rows.get(src_key)
    if row is None:
        return False, "no manifest row for %s 0x%08X" % src_key
    defs, rodata, other = twins.parse_extra(row[4])
    if rodata is not None:
        return False, "source uses --rodata (not ported)"
    cpath = os.path.join(REPO, "re", "match", row[2])
    text = open(cpath, encoding="utf-8").read()
    src_va = int(re.search(r"func_([0-9A-Fa-f]{8})\.c$", row[2]).group(1), 16)
    rw = words_of(src_key[0], src_key[1], sz)
    mw = words_of(unit, va, sz)
    rmask, rslots = twins.mask_function(rw, src_key[1])
    mmask, mslots = twins.mask_function(mw, va)
    if [k for _i, k, _a in rslots] != [k for _i, k, _a in mslots]:
        return False, "relocation slots differ"
    regs = sum(1 for a, b in zip(rmask, mmask) if a != b and (a ^ b) & 0xFFFF == 0)
    if regs:
        return False, "%d word(s) differ in registers" % regs
    files = twins.row_tables(other)
    if files not in tsyms_cache:
        tsyms_cache[files] = twins.table_syms(files)
    tsyms = tsyms_cache[files]
    # name -> address in the source's unit: auto names, then table names, then the row's defsyms
    code = re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)
    addr = {}
    for m in NAME.finditer(code):
        addr[m.group(0)] = int(m.group(2), 16)
    for n, a in twins.code_names(cpath, tsyms):
        addr.setdefault(n, a)
    for n, a in defs:
        addr[n] = a
    own = "func_%08X" % src_va
    addr.pop(own, None)
    moved = dict(twins.wide_moves(sorted(addr.items()), rslots, mslots))
    text = re.sub(r"\b%s\b" % own, "func_%08X" % va, text)
    defsyms = [(n, a) for n, a in sorted(moved.items()) if a != addr[n] or n in dict(defs)]
    ok, res = try_c(out, unit, va, text, defsyms)
    if ok:
        return True, defsyms
    pairs = imm_pairs(rmask, mmask)        # relocation slots are masked out
    if not pairs or len(pairs) > 4:
        return False, "%s; %d immediate pair(s)" % (res, len(pairs))
    # one edit per immediate pair; try combinations, smallest first
    cands = [literal_edits(text, a, b) for a, b in pairs]
    if not all(cands):
        return False, "%s; no literal near 0x%X in the C" % (res, pairs[[bool(c) for c in cands].index(False)][0])
    combos = [[]]
    for c in cands:
        combos = [k + [e] for k in combos for e in c][:max_edits]
    for k in combos:
        edits = sorted({sp for spans in k for sp in spans}, reverse=True)
        if any(edits[i][0] < edits[i + 1][1] for i in range(len(edits) - 1)):
            continue
        t = text
        for s, e, ns in edits:
            t = t[:s] + ns + t[e:]
        ok, res = try_c(out, unit, va, t, defsyms)
        if ok:
            return True, defsyms
    return False, "%s after %d edit set(s)" % (res, len(combos))


def cmd_port(args):
    opn, done = inventory()
    want = None
    if args.items:
        want = {(i.split(":")[0], int(i.split(":")[1], 16)) for i in args.items}
    by = defaultdict(list)
    for u, va, sz in done:
        by[(sz, shape(words_of(u, va, sz)))].append((u, va))
    rows = manifest_rows()
    cache = {}
    for u, va, sz in opn:
        if want is not None and (u, va) not in want:
            continue
        srcs = by.get((sz, shape(words_of(u, va, sz))), [])
        if not srcs:
            continue
        for src in srcs:
            ok, info = port_one(os.path.abspath(args.out), u, va, sz, src, rows, cache, args.max_edits)
            if ok:
                print("MATCH %s 0x%08X from %s 0x%08X  item: %s:0x%08X%s"
                      % (u, va, src[0], src[1], u, va, "".join("@%s=0x%08X" % d for d in info)))
                break
            print("miss  %s 0x%08X from %s 0x%08X: %s" % (u, va, src[0], src[1], info))
        sys.stdout.flush()
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("scan", help="families of open near-copies")
    s.add_argument("--min", type=float, default=0.97, help="shape similarity to call two bodies copies")
    s = sub.add_parser("port", help="port matched C onto open copies with the same shape")
    s.add_argument("out")
    s.add_argument("items", nargs="*", help="UNIT:VADDR to limit the run")
    s.add_argument("--max-edits", type=int, default=40)
    args = p.parse_args(argv)
    return cmd_scan(args) if args.cmd == "scan" else cmd_port(args)


if __name__ == "__main__":
    sys.exit(main())
