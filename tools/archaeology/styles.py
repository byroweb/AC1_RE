#!/usr/bin/env python3
"""styles.py - per-TU asm style fingerprints and how many styles they support.

    python tools/archaeology/styles.py [--gamma 1.0] [--min-insns 1000] [--kmax 5]
                                       [--perms 2000] [--out tools/archaeology/tu_styles.tsv]

Segments every program unit with tus.py, sums arch.py's per-function counts per
TU, turns them into rates, and clusters the TUs of exe / ovl201 / ovl202 that
have at least --min-insns instructions (per-TU rates of small TUs are noise;
they are assigned to the nearest medoid afterwards).  203/204 TUs are twins of
202 TUs and would only duplicate its points, so they are assigned, not
clustered.  Two extra points summarise the mission overlays: the shared
objective-object library (bodies present in >= 10 entries) and the
mission-specific code (bodies present in exactly one entry).

Two feature sets are kept apart on purpose:

  STYLE   habits a programmer carries from file to file whatever the subject:
          short locals (sll/sra 16 pairs), char locals (sll/sra 24), u16/u8
          masking (andi 0xffff / 0xff), signed vs unsigned loads, switch tables
          vs compare chains, indirect method calls, stack traffic outside the
          prologue, saved registers per function, unfilled delay slots,
          function length.
  SUBJECT what the file is about: GTE ops, mult/div, memory ops and calls per
          insn, loops, lwl/lwr struct copies, $gp, frame pointer.

Method: robust z-scores (median / 1.4826*MAD, clipped to +-4) on STYLE, PAM
(k-medoids) for k = 1..--kmax with silhouette per k, and a permutation test of
the ovl201-vs-ovl202 split (between/within distance ratio against --perms
random relabellings of the same sizes).
"""
import argparse
import math
import os
import random
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import arch  # noqa: E402
import tus as tusmod  # noqa: E402

OUT = os.path.join(HERE, "tu_styles.tsv")


def rates(c):
    n = max(c["n"], 1)
    mem = max(c["ld_b"] + c["ld_bu"] + c["ld_h"] + c["ld_hu"] + c["ld_w"]
              + c["st_b"] + c["st_h"] + c["st_w"], 1)
    lh = c["ld_h"] + c["ld_hu"]
    lb = c["ld_b"] + c["ld_bu"]
    nf = max(c["nf"], 1)
    k = 1000.0 / n
    return {
        # STYLE
        "sext16": c["sext16"] * k,
        "sext8": c["sext8"] * k,
        "zext16": c["zext16"] * k,
        "zext8": c["zext8"] * k,
        "h_signed": c["ld_h"] / lh if lh else 0.5,
        "b_signed": c["ld_b"] / lb if lb else 0.5,
        "byte_share": (lb + c["st_b"]) / mem,
        "half_share": (lh + c["st_h"]) / mem,
        "jt_per_cmp": c["jt"] / (c["jt"] + c["li_cmp"]) if (c["jt"] + c["li_cmp"]) else 0.0,
        "li_cmp": c["li_cmp"] * k,
        "vcall": c["vcall"] * k,
        "sp_traffic": (c["sp_ld"] + c["sp_st"]) * k,
        "saved_per_fn": c["saved"] / nf,
        "dslot_nop": c["dnop"] / c["dslots"] if c["dslots"] else 0.0,
        "insns_per_fn": c["n"] / nf,
        "narrow_inc": (c["inc16"] + c["inc8"]) / c["inc"] if c["inc"] else 0.0,
        # SUBJECT
        "gte": c["gte"] * k,
        "muldiv": (c["mul"] + c["div"]) * k,
        "mem": mem * k,
        "calls": c["jal"] * k,
        "loops": c["bwd"] * k,
        "lwlr": c["lwlr"] * k,
        "leaf_share": c["leaf"] / nf,
        "gp": c["gp"] * k,
        "fp_fns": c["fp"] / nf,
    }


STYLE = ["sext16", "sext8", "zext16", "zext8", "h_signed", "b_signed", "byte_share",
         "half_share", "jt_per_cmp", "li_cmp", "vcall", "sp_traffic", "saved_per_fn",
         "dslot_nop", "insns_per_fn", "narrow_inc"]
SUBJECT = ["gte", "muldiv", "mem", "calls", "loops", "lwlr", "leaf_share", "gp", "fp_fns"]


def sum_counts(frows):
    c = defaultdict(int)
    for r in frows:
        for k in arch.FEATURES:
            c[k] += r["f"][k]
        c["nf"] += 1
    return c


def collect(gamma, units=arch.PROGRAM_UNITS):
    allt = []
    for unit in units:
        rows, tus, _ = tusmod.segment(unit, gamma)
        for t in tus:
            t["counts"] = sum_counts(rows[t["i0"]:t["i1"]])
            t["rates"] = rates(t["counts"])
            allt.append(t)
    return allt


def mission_points():
    """Two pseudo-TUs: mission library bodies (>= 10 entries) and mission-specific bodies."""
    import twins
    rows = [r for r in arch.load_rows() if r["unit"].startswith("fdat")]
    by = defaultdict(set)
    for r in rows:
        r["f"] = arch.features(r["unit"], r)
        m, _ = twins.mask_function(twins.func_words(r["unit"], r["va"], r["sz"]), r["va"])
        r["m"] = m
        by[m].add(r["unit"])
    seen, lib, own = set(), [], []
    for r in rows:
        if r["m"] in seen:
            continue
        seen.add(r["m"])
        n = len(by[r["m"]])
        (lib if n >= 10 else own if n == 1 else []).append(r)
    out = []
    for name, grp in (("fdat.library", lib), ("fdat.specific", own)):
        c = sum_counts(grp)
        out.append({"unit": "fdat", "tu": name, "start": 0, "end": 0, "nfunc": len(grp),
                    "counts": c, "rates": rates(c)})
    return out


def robust_scale(points, keys):
    med, mad = {}, {}
    for k in keys:
        v = sorted(p[k] for p in points)
        med[k] = v[len(v) // 2]
        d = sorted(abs(x - med[k]) for x in v)
        mad[k] = 1.4826 * d[len(d) // 2] or (max(v) - min(v)) / 4 or 1.0

    def z(p):
        return [max(-4.0, min(4.0, (p[k] - med[k]) / mad[k])) for k in keys]
    return z


def dist(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b)))


def pam(D, k, rng, restarts=20):
    n = len(D)
    best = None
    for _ in range(restarts):
        med = rng.sample(range(n), k)
        while True:
            cost = sum(min(D[i][m] for m in med) for i in range(n))
            improved = False
            for mi in range(k):
                for h in range(n):
                    if h in med:
                        continue
                    trial = med[:mi] + [h] + med[mi + 1:]
                    c = sum(min(D[i][m] for m in trial) for i in range(n))
                    if c < cost - 1e-12:
                        med, cost, improved = trial, c, True
            if not improved:
                break
        if best is None or cost < best[0]:
            best = (cost, sorted(med))
    med = best[1]
    lab = [min(range(k), key=lambda j: D[i][med[j]]) for i in range(n)]
    return med, lab, best[0]


def silhouette(D, lab):
    ks = set(lab)
    if len(ks) < 2:
        return 0.0
    s = []
    for i in range(len(D)):
        own = [D[i][j] for j in range(len(D)) if j != i and lab[j] == lab[i]]
        if not own:
            s.append(0.0)
            continue
        a = sum(own) / len(own)
        b = min(sum(D[i][j] for j in range(len(D)) if lab[j] == k)
                / sum(1 for j in range(len(D)) if lab[j] == k) for k in ks if k != lab[i])
        s.append((b - a) / max(a, b))
    return sum(s) / len(s)


def split_ratio(D, lab):
    """mean between-group distance / mean within-group distance."""
    bw, nb, wi, nw = 0.0, 0, 0.0, 0
    for i in range(len(D)):
        for j in range(i + 1, len(D)):
            if lab[i] == lab[j]:
                wi += D[i][j]
                nw += 1
            else:
                bw += D[i][j]
                nb += 1
    return (bw / nb) / (wi / nw) if nb and nw else 0.0


REGIONS = [
    ("exe game code", "exe", 0x80011F28, 0x80016EA4),
    ("ovl201 all", "ovl201", 0x8004DCC0, 0x800B597C),
    ("ovl201 0x8004DCC0-0x800866FC", "ovl201", 0x8004DCC0, 0x800866FC),
    ("ovl201 0x800866FC-0x8009A7E8", "ovl201", 0x800866FC, 0x8009A7E8),
    ("ovl201 0x8009A7E8-0x800B597C", "ovl201", 0x8009A7E8, 0x800B597C),
    ("ovl201 0x8005B12C-0x8005E710", "ovl201", 0x8005B12C, 0x8005E710),
    ("ovl202 all", "ovl202", 0x8004C1BC, 0x8008D91C),
    ("ovl202 0x8004C1BC-0x80073C2C", "ovl202", 0x8004C1BC, 0x80073C2C),
    ("ovl202 0x80073C2C-0x8008A048", "ovl202", 0x80073C2C, 0x8008A048),
    ("ovl202 0x8008A048-0x8008D91C", "ovl202", 0x8008A048, 0x8008D91C),
]


def regions(spec=REGIONS):
    """Aggregate STYLE rates over fixed address ranges (whole-region sums, not TU means)."""
    cache = {}
    keys = ["narrow_inc", "sext16", "zext8", "zext16", "b_signed", "h_signed", "byte_share",
            "vcall", "jt_per_cmp", "sp_traffic", "saved_per_fn", "dslot_nop", "insns_per_fn", "gte"]
    print("%-30s %5s %6s " % ("region", "fns", "insns") + " ".join("%7s" % k[:7] for k in keys))
    for name, unit, lo, hi in spec:
        if unit not in cache:
            cache[unit] = [r for r in arch.load_rows([unit]) if arch.is_game(r)]
            for r in cache[unit]:
                r["f"] = arch.features(unit, r)
        rs = [r for r in cache[unit] if lo <= r["va"] < hi]
        c = sum_counts(rs)
        rt = rates(c)
        print("%-30s %5d %6d " % (name, c["nf"], c["n"]) + " ".join("%7.3f" % rt[k] for k in keys))


def main(argv=None):
    if argv is None and "--regions" in sys.argv:
        regions()
        return
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--regions", action="store_true",
                    help="print STYLE rates over the fixed address regions used in the doc")
    ap.add_argument("--gamma", type=float, default=1.0)
    ap.add_argument("--min-insns", type=int, default=1000)
    ap.add_argument("--kmax", type=int, default=5)
    ap.add_argument("--k", type=int, default=0, help="report this k (default: best silhouette)")
    ap.add_argument("--perms", type=int, default=2000)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args(argv)
    rng = random.Random(a.seed)
    allt = collect(a.gamma) + mission_points()
    core = [t for t in allt if t["unit"] in ("exe", "ovl201", "ovl202")
            and t["counts"]["n"] >= a.min_insns]
    z = robust_scale([t["rates"] for t in core], STYLE)
    X = [z(t["rates"]) for t in core]
    D = [[dist(x, y) for y in X] for x in X]
    print("%d core TUs (>= %d insns): %s" % (len(core), a.min_insns, dict(
        (u, sum(1 for t in core if t["unit"] == u)) for u in ("exe", "ovl201", "ovl202"))),
        file=sys.stderr)

    # 201 vs 202 permutation test (exe TUs excluded: too few to matter)
    sub = [i for i, t in enumerate(core) if t["unit"] in ("ovl201", "ovl202")]
    Ds = [[D[i][j] for j in sub] for i in sub]
    lab = [core[i]["unit"] == "ovl201" for i in sub]
    obs = split_ratio(Ds, lab)
    ge = sum(1 for _ in range(a.perms)
             if split_ratio(Ds, rng.sample(lab, len(lab))) >= obs)
    print("ovl201 vs ovl202 between/within = %.3f, permutation p = %.4f (%d perms)"
          % (obs, (ge + 1) / (a.perms + 1), a.perms), file=sys.stderr)
    for kk in STYLE:
        v1 = sorted(core[i]["rates"][kk] for i in sub if lab[sub.index(i)])
        v2 = sorted(core[i]["rates"][kk] for i in sub if not lab[sub.index(i)])
        print("   %-13s median 201 %8.3f   202 %8.3f" % (kk, v1[len(v1) // 2], v2[len(v2) // 2]),
              file=sys.stderr)

    sil, fits = {}, {}
    for k in range(1, a.kmax + 1):
        med, labk, cost = pam(D, k, rng)
        fits[k] = (med, labk)
        sil[k] = silhouette(D, labk) if k > 1 else 0.0
        comp = defaultdict(lambda: defaultdict(int))
        for t, l in zip(core, labk):
            comp[l][t["unit"]] += 1
        print("k=%d cost %.1f silhouette %.3f  %s" % (k, cost, sil[k],
              [dict(comp[l]) for l in range(k)]), file=sys.stderr)
    k = a.k or max(range(2, a.kmax + 1), key=lambda kk: sil[kk])
    med, labk = fits[k]
    for t in allt:
        zt = z(t["rates"])
        t["cluster"] = min(range(k), key=lambda j: dist(zt, X[med[j]]))
        t["core"] = 0
    for t, l in zip(core, labk):
        t["cluster"], t["core"] = l, 1
    print("chosen k=%d; medoids %s" % (k, [core[m]["tu"] for m in med]), file=sys.stderr)
    for l in range(k):
        mem = [t for t in allt if t["cluster"] == l and t["unit"] in ("exe", "ovl201", "ovl202")]
        n = sum(t["counts"]["n"] for t in mem)
        units = defaultdict(int)
        for t in mem:
            units[t["unit"]] += t["counts"]["n"]
        print("cluster %s: %d TUs, %d insns, by unit %s" % ("ABCDEFGH"[l], len(mem), n, dict(units)),
              file=sys.stderr)
    for t in allt:
        if t["unit"] == "fdat":
            print("%s -> cluster %s" % (t["tu"], "ABCDEFGH"[t["cluster"]]), file=sys.stderr)
    with open(a.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\ttu\tstart\tend\tfunctions\tinsns\tcluster\tcore\t"
                 + "\t".join(STYLE + SUBJECT) + "\n")
        for t in allt:
            fh.write("%s\t%s\t0x%08X\t0x%08X\t%d\t%d\t%s\t%d\t%s\n" % (
                t["unit"], t["tu"], t["start"], t["end"], t["nfunc"], t["counts"]["n"],
                "ABCDEFGH"[t["cluster"]], t["core"],
                "\t".join("%.3f" % t["rates"][kk] for kk in STYLE + SUBJECT)))
    print("wrote %s" % a.out, file=sys.stderr)


if __name__ == "__main__":
    main()
