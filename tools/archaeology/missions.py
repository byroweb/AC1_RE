#!/usr/bin/env python3
"""missions.py - lineage of the 47 story mission units (FDAT_T entries 2N).

    python tools/archaeology/missions.py [--extra] [--out tools/archaeology/mission_lineage.tsv]

For every story unit (fdat000..fdat098, even, without 032/058/078; --extra adds
fdat120/fdat140) it reads the functions of tools/loop/functions.tsv and builds:

  bodies      set of relocation-masked bodies (twins.mask_function), i.e. the
              same identity twins.py uses for replay.
  families    coarse identity: opcode/register skeleton with every immediate
              masked.  One family with several bodies = the same source
              function edited (offsets, constants, frame) between missions.
  template    calls into ovl202: LOS (0x800781E8 has_los, 0x8007816C los_ray),
              aim cone 0x80073C2C, acquire 0x80073714, plus text size,
              function count, jump tables, the 4912-byte library dispatcher.

Distances: 1 - weighted Jaccard over bodies (weight = body bytes).  The tree
is UPGMA (pure Python) and printed as an indented dendrogram and as Newick.
Clone edges: for each unit, the unit with which it shares the largest share of
its *mission-specific* bytes (bodies present in <= 3 units), and the variant
characters (family -> body) it shares only with that unit.

Ordering evidence for "earliest template": a unit whose variant of a family is
the one most other units carry is conservative; a unit carrying many unique
variants of shared families is derived.  The script prints, per unit, the share
of its multi-unit families where it carries the majority variant.
"""
import argparse
import os
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import arch  # noqa: E402
import twins  # noqa: E402  (tools/loop is on sys.path via arch)

OUT = os.path.join(HERE, "mission_lineage.tsv")
STORY = ["fdat%03d" % e for e in range(0, 100, 2) if e not in (32, 58, 78)]
EXTRA = ["fdat120", "fdat140"]
CALLS = {"has_los": 0x800781E8, "los_ray": 0x8007816C, "cone": 0x80073C2C,
         "acquire": 0x80073714}


def coarse(words):
    out = []
    for w in words:
        op = w >> 26
        out.append(w & 0xFC000000 if op in (2, 3) else w if op == 0 else w & 0xFFFF0000)
    return tuple(out)


def load(units):
    rows = [r for r in arch.load_rows(units)]
    per = defaultdict(list)
    for r in rows:
        words = twins.func_words(r["unit"], r["va"], r["sz"])
        r["m"], r["slots"] = twins.mask_function(words, r["va"])
        r["c"] = coarse(words)
        per[r["unit"]].append(r)
    return per


def template(rs):
    t = Counter()
    for r in rs:
        for (_, k, tgt) in r["slots"]:
            if k == "J":
                for name, a in CALLS.items():
                    if tgt == a:
                        t[name] += 1
        t["jt"] += int(r["jump_tables"] or 0)
        if r["sz"] == 4912:
            t["disp4912"] += 1
    t["text"] = sum(r["sz"] for r in rs)
    t["nfunc"] = len(rs)
    return t


def upgma(names, D):
    clusters = {i: (names[i], 1, 0.0) for i in range(len(names))}   # newick, size, height
    d = {(i, j): D[i][j] for i in range(len(names)) for j in range(i + 1, len(names))}
    nxt = len(names)
    merges = []
    while len(clusters) > 1:
        (a, b), h = min(d.items(), key=lambda kv: kv[1])
        na, sa, ha = clusters.pop(a)
        nb, sb, hb = clusters.pop(b)
        node = "(%s:%.3f,%s:%.3f)" % (na, h / 2 - ha, nb, h / 2 - hb)
        newd = {}
        for (x, y), v in d.items():
            if a in (x, y) or b in (x, y):
                continue
            newd[(x, y)] = v
        for c in clusters:
            da = d[(min(a, c), max(a, c))]
            db = d[(min(b, c), max(b, c))]
            newd[(min(c, nxt), max(c, nxt))] = (da * sa + db * sb) / (sa + sb)
        d = newd
        clusters[nxt] = (node, sa + sb, h / 2)
        merges.append((a, b, h, nxt))
        nxt += 1
    return list(clusters.values())[0][0] + ";", merges


def leaves_of(merges, n):
    kids = {}
    for a, b, h, c in merges:
        kids[c] = (a, b, h)

    def lv(x):
        if x < n:
            return [x]
        a, b, _ = kids[x]
        return lv(a) + lv(b)
    return kids, lv


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--extra", action="store_true", help="include fdat120 / fdat140")
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args(argv)
    units = STORY + (EXTRA if a.extra else [])
    per = load(units)
    names = [u for u in units if u in per]
    body_units = defaultdict(set)
    body_size = {}
    fam_bodies = defaultdict(lambda: defaultdict(set))   # family -> body -> units
    for u in names:
        for r in per[u]:
            body_units[r["m"]].add(u)
            body_size[r["m"]] = r["sz"]
            fam_bodies[r["c"]][r["m"]].add(u)
    sets = {u: {r["m"] for r in per[u]} for u in names}

    def wj(x, y):
        inter = sum(body_size[b] for b in sets[x] & sets[y])
        union = sum(body_size[b] for b in sets[x] | sets[y])
        return 1.0 - inter / union if union else 0.0
    D = [[wj(x, y) for y in names] for x in names]
    tmpl = {u: template(per[u]) for u in names}
    mnum = {u: int(u[4:]) // 2 for u in names}
    label = {u: "M%02d" % mnum[u] for u in names}

    # majority-variant share: conservative (ancestral-looking) vs derived units
    cons = {}
    uniq = Counter()
    for fam, bodies in fam_bodies.items():
        if len(bodies) < 2:
            continue
        tot = sum(len(v) for v in bodies.values())
        if tot < 3:
            continue
        maj = max(bodies.values(), key=len)
        for b, us in bodies.items():
            for u in us:
                cons.setdefault(u, [0, 0])
                cons[u][1] += 1
                if us is maj:
                    cons[u][0] += 1
                if len(us) == 1:
                    uniq[u] += 1

    # clone edges on mission-specific bytes (bodies in <= 3 units)
    clone = {}
    for u in names:
        spec = {b for b in sets[u] if len(body_units[b]) <= 3}
        tot = sum(body_size[b] for b in spec)
        best = None
        for v in names:
            if v == u:
                continue
            sh = sum(body_size[b] for b in spec & sets[v])
            if best is None or sh > best[1]:
                best = (v, sh)
        clone[u] = (best[0], best[1] / tot if tot else 0.0, tot)

    newick, merges = upgma([label[u] for u in names], D)
    kids, lv = leaves_of(merges, len(names))

    def show(x, depth):
        if x < len(names):
            u = names[x]
            t = tmpl[u]
            print("%s%s %-7s fn=%2d text=%5d los=%d/%d cone=%d acq=%d disp4912=%d  clone<-%s (%.0f%% of %d B)"
                  % ("  " * depth, label[u], u, t["nfunc"], t["text"], t["has_los"], t["los_ray"],
                     t["cone"], t["acquire"], t["disp4912"], label[clone[u][0]],
                     100 * clone[u][1], clone[u][2]))
            return
        a_, b_, h = kids[x]
        print("%s+ d=%.2f" % ("  " * depth, h))
        show(a_, depth + 1)
        show(b_, depth + 1)
    show(merges[-1][3], 0)
    print("\nNewick:", newick)

    # mission-number adjacency vs similarity
    adj = [D[i][i + 1] for i in range(len(names) - 1)]
    allp = [D[i][j] for i in range(len(names)) for j in range(i + 1, len(names))]
    print("\nmean distance, consecutive mission numbers %.3f vs all pairs %.3f"
          % (sum(adj) / len(adj), sum(allp) / len(allp)))
    # the two top-level families and their mission numbers (story order as a dev-order proxy)
    root = merges[-1][3]
    fam = [sorted(mnum[names[i]] for i in lv(kids[root][0])),
           sorted(mnum[names[i]] for i in lv(kids[root][1]))]
    allm = fam[0] + fam[1]
    obs = abs(sum(fam[0]) / len(fam[0]) - sum(fam[1]) / len(fam[1]))
    import random
    rng = random.Random(1)
    ge = 0
    for _ in range(10000):
        p = rng.sample(allm, len(allm))
        a_, b_ = p[:len(fam[0])], p[len(fam[0]):]
        if abs(sum(a_) / len(a_) - sum(b_) / len(b_)) >= obs:
            ge += 1
    for f in fam:
        print("family of %d: missions %s (median %d)" % (len(f), f, f[len(f) // 2]))
    print("difference of mean mission number %.1f, permutation p = %.4f" % (obs, (ge + 1) / 10001.0))
    print("\nconservative share (majority variant of shared families), most conservative first:")
    for u in sorted(names, key=lambda u: -cons.get(u, [0, 1])[0] / max(cons.get(u, [0, 1])[1], 1)):
        c = cons.get(u, [0, 0])
        print("  %s %-7s %3d/%3d = %.2f  unique variants %d" % (
            label[u], u, c[0], c[1], c[0] / c[1] if c[1] else 0, uniq[u]))
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\tmission\tfunctions\ttext_bytes\thas_los\tlos_ray\tcone\tacquire\tjump_tables"
                 "\tdisp4912\tclone_of\tclone_share\tmajority_share\tunique_variants\n")
        for u in names:
            t = tmpl[u]
            c = cons.get(u, [0, 0])
            fh.write("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%.2f\t%.2f\t%d\n" % (
                u, mnum[u], t["nfunc"], t["text"], t["has_los"], t["los_ray"], t["cone"],
                t["acquire"], t["jt"], t["disp4912"], clone[u][0], clone[u][1],
                c[0] / c[1] if c[1] else 0, uniq[u]))
        fh.write("# newick (UPGMA on 1 - byte-weighted Jaccard of masked bodies)\n# %s\n" % newick)
    print("wrote %s" % a.out, file=sys.stderr)


if __name__ == "__main__":
    main()
