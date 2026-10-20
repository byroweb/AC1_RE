#!/usr/bin/env python3
"""tus.py - infer translation-unit (object file) boundaries from link order.

    python tools/archaeology/tus.py [--units exe,ovl201,...] [--gamma 1.0]
                                    [--kmax 12] [--out tools/archaeology/tu_map.tsv]

No overlay has inter-function nop padding (0 of 2871 overlay functions end in
padding; psylink packs .text at 4-byte alignment), so object-file boundaries
are invisible in the bytes and have to be inferred from reference locality:

1. Graph.  Nodes are the unit's game functions in address order.  Edges:
   every in-unit call (jal) or code pointer (lui/addiu pair into .text) links
   caller and callee (weight 1 per pair); every lui/lo address into the
   unit's own .rodata / .data / .bss that is referenced by 2..--kmax functions
   links those functions (total weight per address ~ |S|/2).  Addresses used
   by more functions are shared globals and are skipped.
2. Cut.  The contiguous partition of the function list that maximises
   Newman modularity (resolution --gamma) is found exactly by dynamic
   programming (O(n^2)).  Static helpers, string literals, jump tables and
   file-scope statics pull a file's functions together; calls to other
   files' globals are discounted by the null model.
3. Check.  Each TU's rodata range must not interleave with its neighbour's
   (a TU owns one contiguous .rodata block in link order); the count of
   interleaves is printed.  The share of edge weight that stays inside a TU
   is printed too.

Output: one row per TU (unit, tu, start, end exclusive, functions, bytes,
referenced rodata / data ranges).  All of it is HYPOTHESIS-grade: see
docs/CODE_ARCHAEOLOGY.md for how far the cuts can be trusted.
"""
import argparse
import bisect
import os
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import arch  # noqa: E402

OUT = os.path.join(HERE, "tu_map.tsv")


def unit_functions(unit):
    rows = [r for r in arch.load_rows([unit]) if arch.is_game(r)]
    rows.sort(key=lambda r: r["va"])
    for r in rows:
        r["f"] = arch.features(unit, r)
    return rows


def references(unit, rows):
    """{object key: set(function index)} and {function index: {section: [addr]}}."""
    starts = [r["va"] for r in rows]

    def owner(addr):
        k = bisect.bisect_right(starts, addr) - 1
        return k if k >= 0 else None

    refs = defaultdict(set)
    data = defaultdict(lambda: defaultdict(list))
    for i, r in enumerate(rows):
        for (_, kind, tgt) in r["f"]["slots"]:
            if kind in ("HI", "HI*"):
                continue
            sec = arch.section_of(unit, tgt)
            if sec is None:
                continue
            if sec == "text":
                k = owner(tgt)
                if k is None or k == i:
                    continue
                refs[("fn", k)].update((i, k))
            else:
                refs[(sec, tgt)].add(i)
                data[i][sec].append(tgt)
    return refs, data


def graph(n, refs, kmax):
    """Weighted reference graph: ({j: [(i, w)]} with i < j, degrees, total weight)."""
    w = defaultdict(float)
    for key, s in refs.items():
        s = sorted(s)
        if len(s) < 2:
            continue
        if key[0] == "fn":
            k = key[1]
            for i in s:
                if i != k:
                    w[(min(i, k), max(i, k))] += 1.0
        elif len(s) <= kmax:
            x = 1.0 / (len(s) - 1)
            for p in range(len(s)):
                for q in range(p + 1, len(s)):
                    w[(s[p], s[q])] += x
    before = defaultdict(list)
    deg = [0.0] * n
    for (i, j), x in w.items():
        before[j].append((i, x))
        deg[i] += x
        deg[j] += x
    return before, deg, sum(w.values())


def modularity_cuts(n, before, deg, m, gamma):
    """Contiguous partition of 0..n-1 maximising modularity (exact DP, O(n^2))."""
    if m == 0:
        return [0, n]
    pd = [0.0] * (n + 1)
    for i in range(n):
        pd[i + 1] = pd[i] + deg[i]
    col = [0.0] * (n + 1)      # col[a] = sum of w(i,j) with i < a, j < b (current b)
    best = [0.0] + [None] * n
    arg = [0] * (n + 1)
    m2 = 2.0 * m
    for b in range(1, n + 1):
        inc = [0.0] * (n + 1)
        for i, x in before.get(b - 1, ()):
            inc[i + 1] += x
        run = 0.0
        for a in range(n + 1):
            run += inc[a]
            col[a] += run
        tot = col[b]
        bv, ba = None, 0
        pb = pd[b]
        for a in range(b):
            d = (pb - pd[a]) / m2
            v = best[a] + (tot - col[a]) / m - gamma * d * d
            if bv is None or v > bv:
                bv, ba = v, a
        best[b], arg[b] = bv, ba
    cuts = [n]
    while cuts[-1]:
        cuts.append(arg[cuts[-1]])
    return cuts[::-1]


def tu_table(unit, rows, cuts, data):
    tus = []
    for t in range(len(cuts) - 1):
        a, b = cuts[t], cuts[t + 1]
        secs = defaultdict(list)
        for i in range(a, b):
            for sec, v in data[i].items():
                secs[sec].extend(v)
        last = rows[b - 1]
        tus.append({
            "unit": unit, "tu": "%s.t%02d" % (unit, t),
            "start": rows[a]["va"], "end": last["va"] + last["sz"],
            "nfunc": b - a, "bytes": last["va"] + last["sz"] - rows[a]["va"],
            "rodata": (min(secs["rodata"]), max(secs["rodata"])) if secs["rodata"] else None,
            "data": (min(secs["data"]), max(secs["data"])) if secs["data"] else None,
            "i0": a, "i1": b,
        })
    return tus


def rodata_interleaves(tus):
    """TU pairs (t, next t with rodata) whose rodata ranges interleave."""
    bad, prev = [], None
    for t in tus:
        if t["rodata"] is None:
            continue
        if prev and t["rodata"][0] < prev["rodata"][1]:
            bad.append((prev["tu"], t["tu"]))
        prev = t
    return bad


def segment(unit, gamma=1.0, kmax=12):
    """(rows, tus, internal edge share)."""
    rows = unit_functions(unit)
    refs, data = references(unit, rows)
    before, deg, m = graph(len(rows), refs, kmax)
    cuts = modularity_cuts(len(rows), before, deg, m, gamma)
    tu_of = {}
    for t in range(len(cuts) - 1):
        for i in range(cuts[t], cuts[t + 1]):
            tu_of[i] = t
    internal = sum(x for j, lst in before.items() for i, x in lst if tu_of[i] == tu_of[j])
    for i, r in enumerate(rows):
        r["tu"] = tu_of[i]
    return rows, tu_table(unit, rows, cuts, data), (internal / m if m else 1.0)


LABELS = os.path.join(HERE, "doc_labels.tsv")


def doc_labels(path=LABELS):
    """{unit: [(addr, role, system)]} from doc_labels.tsv."""
    out = defaultdict(list)
    if not os.path.exists(path):
        return out
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or line.startswith("unit\t"):
                continue
            p = line.rstrip("\n").split("\t")
            if len(p) >= 4:
                out[p[0]].append((int(p[1], 16), p[2], p[3]))
    return out


def tu_systems(t, labels):
    """'system xN, ...' for the doc labels that fall inside TU t."""
    c = defaultdict(int)
    for addr, role, system in labels.get(t["unit"], ()):
        if t["start"] <= addr < t["end"]:
            c[system] += 1
    return ", ".join("%s x%d" % kv if kv[1] > 1 else kv[0]
                     for kv in sorted(c.items(), key=lambda kv: -kv[1]))


def fmt_rng(r):
    return "" if r is None else "0x%08X-0x%08X" % r


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--units", default=",".join(arch.PROGRAM_UNITS))
    ap.add_argument("--gamma", type=float, default=1.0,
                    help="modularity resolution (higher = smaller TUs)")
    ap.add_argument("--kmax", type=int, default=12,
                    help="data addresses used by more functions than this are shared globals")
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args(argv)
    allt = []
    for unit in a.units.split(","):
        rows, tus, frac = segment(unit, a.gamma, a.kmax)
        bad = rodata_interleaves(tus)
        print("%-7s %4d functions -> %3d TUs  internal edge weight %.2f  rodata interleaves %d %s"
              % (unit, len(rows), len(tus), frac, len(bad), bad[:4]), file=sys.stderr)
        allt.extend(tus)
    labels = doc_labels()
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\ttu\tstart\tend\tfunctions\tbytes\trodata\tdata\tdoc_systems\n")
        for t in allt:
            fh.write("%s\t%s\t0x%08X\t0x%08X\t%d\t%d\t%s\t%s\t%s\n" % (
                t["unit"], t["tu"], t["start"], t["end"], t["nfunc"], t["bytes"],
                fmt_rng(t["rodata"]), fmt_rng(t["data"]), tu_systems(t, labels)))
    print("wrote %s (%d TUs)" % (a.out, len(allt)), file=sys.stderr)


if __name__ == "__main__":
    main()
