#!/usr/bin/env python3
"""refit.py - propose new rank.py WEIGHTS from the ledger outcomes.

    refit.py                               # tools/loop/ledger.tsv
    refit.py --ledger tools/loop/ledger-A.tsv --ledger tools/loop/ledger-B.tsv
    refit.py --giveup-attempts 20 --lam 1.0 --top 20

Reads every (unit, vaddr) outcome from the ledger(s), looks the function up in
functions.tsv and fits

    log(attempts) ~ b0 + sum_k b_k * feature_k

with ridge regression (closed form, pure Python; numpy is not needed) on the
same features rank.score() uses: log1p(instructions), log1p(branches),
log1p(gte_ops), log1p(stack_size), log1p(calls) and the flat jump_tables
count.  Give-ups count as --giveup-attempts (default 20: the budget plus a
penalty).  Matches with attempts == 0 (unknown) are skipped.

The fit is reported, never written: it prints

  * the sample (n, attempts histogram, give-ups),
  * Spearman rho of the CURRENT rank.score() against attempts, and of the
    refit prediction (in-sample and leave-one-out),
  * the coefficients mapped back to WEIGHTS space (per unit of log1p feature),
    normalised so instructions = 1.00 as in rank.py, with sign warnings
    (a difficulty feature that comes out negative is clamped to 0; calls is
    the one feature rank.py subtracts, so a positive fitted effect is
    reported and clamped to 0),
  * the current vs proposed top-N of the open candidates.

Edit WEIGHTS in rank.py by hand from that; the sample is small and the
proposal is a guide, not a truth.
"""
import argparse
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ledger  # noqa: E402
import rank    # noqa: E402

LOG_FEATURES = ("instructions", "branches", "gte_ops", "stack_size", "calls")
FLAT_FEATURES = ("jump_tables",)
FEATURES = LOG_FEATURES + FLAT_FEATURES


# ---------------------------------------------------------------------------
# small linear algebra
# ---------------------------------------------------------------------------
def solve(a, b):
    """Gauss-Jordan with partial pivoting; a is n x n (list of lists), b is n."""
    n = len(a)
    m = [row[:] + [b[i]] for i, row in enumerate(a)]
    for col in range(n):
        piv = max(range(col, n), key=lambda r: abs(m[r][col]))
        if abs(m[piv][col]) < 1e-12:
            raise ZeroDivisionError("singular system")
        m[col], m[piv] = m[piv], m[col]
        p = m[col][col]
        m[col] = [v / p for v in m[col]]
        for r in range(n):
            if r != col and m[r][col] != 0.0:
                f = m[r][col]
                m[r] = [rv - f * cv for rv, cv in zip(m[r], m[col])]
    return [m[i][n] for i in range(n)]


def ridge(xs, ys, lam):
    """xs: list of standardised feature rows (no intercept), ys: targets.
    Returns (b0, [b_k]) with the intercept unpenalised."""
    n, k = len(xs), len(xs[0]) if xs else 0
    # augment with a constant column at index 0
    d = k + 1
    xtx = [[0.0] * d for _ in range(d)]
    xty = [0.0] * d
    for x, y in zip(xs, ys):
        row = [1.0] + x
        for i in range(d):
            xty[i] += row[i] * y
            for j in range(d):
                xtx[i][j] += row[i] * row[j]
    for i in range(1, d):
        xtx[i][i] += lam
    beta = solve(xtx, xty)
    return beta[0], beta[1:]


def ranks(values):
    """average ranks for Spearman (ties share the mean rank)."""
    order = sorted(range(len(values)), key=lambda i: values[i])
    out = [0.0] * len(values)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and values[order[j + 1]] == values[order[i]]:
            j += 1
        r = (i + j) / 2.0 + 1.0
        for t in range(i, j + 1):
            out[order[t]] = r
        i = j + 1
    return out


def pearson(a, b):
    n = len(a)
    if n < 3:
        return float("nan")
    ma, mb = sum(a) / n, sum(b) / n
    sa = math.sqrt(sum((x - ma) ** 2 for x in a))
    sb = math.sqrt(sum((x - mb) ** 2 for x in b))
    if sa == 0 or sb == 0:
        return float("nan")
    return sum((x - ma) * (y - mb) for x, y in zip(a, b)) / (sa * sb)


def spearman(a, b):
    return pearson(ranks(a), ranks(b))


# ---------------------------------------------------------------------------
# data
# ---------------------------------------------------------------------------
def feature_vector(row):
    v = [math.log1p(row[k]) for k in LOG_FEATURES]
    v += [float(row[k]) for k in FLAT_FEATURES]
    return v


def collect(ledger_paths, functions, giveup_attempts):
    by_key = {(r["unit"], r["vaddr"]): r for r in functions}
    outcomes = {}
    for p in ledger_paths:
        for key, e in ledger.load(p).items():
            outcomes[key] = e            # later files win, like the ledger itself
    sample, skipped = [], []
    for key, e in outcomes.items():
        r = by_key.get(key)
        if r is None:
            skipped.append((key, "not in functions.tsv"))
            continue
        if e["status"] == "match":
            if e["attempts"] <= 0:
                skipped.append((key, "match with unknown attempts"))
                continue
            y = float(e["attempts"])
        else:
            y = float(giveup_attempts)
        sample.append({"key": key, "row": r, "status": e["status"], "attempts": y})
    return sample, skipped


def standardise(vectors):
    k = len(vectors[0])
    n = len(vectors)
    means = [sum(v[i] for v in vectors) / n for i in range(k)]
    sds = []
    for i in range(k):
        var = sum((v[i] - means[i]) ** 2 for v in vectors) / max(n - 1, 1)
        sds.append(math.sqrt(var))
    keep = [i for i in range(k) if sds[i] > 1e-9]
    xs = [[(v[i] - means[i]) / sds[i] for i in keep] for v in vectors]
    return xs, keep, means, sds


def fit(sample, lam):
    vecs = [feature_vector(s["row"]) for s in sample]
    ys = [math.log(s["attempts"]) for s in sample]
    xs, keep, means, sds = standardise(vecs)
    b0, bs = ridge(xs, ys, lam)
    raw = {}                       # coefficient per unit of the raw feature
    for j, i in enumerate(keep):
        raw[FEATURES[i]] = bs[j] / sds[i]
    pred = [b0 + sum(b * x for b, x in zip(bs, x_)) for x_ in xs]
    return raw, keep, ys, pred, (xs, b0, bs)


def loo_predictions(sample, lam):
    out = []
    for i in range(len(sample)):
        train = sample[:i] + sample[i + 1:]
        vecs = [feature_vector(s["row"]) for s in train]
        ys = [math.log(s["attempts"]) for s in train]
        xs, keep, means, sds = standardise(vecs)
        try:
            b0, bs = ridge(xs, ys, lam)
        except ZeroDivisionError:
            out.append(float("nan"))
            continue
        v = feature_vector(sample[i]["row"])
        x = [(v[k] - means[k]) / sds[k] for k in keep]
        out.append(b0 + sum(b * xx for b, xx in zip(bs, x)))
    return out


def propose_weights(raw, keep):
    """Map fitted raw coefficients to a WEIGHTS dict like rank.WEIGHTS."""
    cur = rank.WEIGHTS
    notes = []
    w = dict(cur)
    inst = raw.get("instructions")
    if inst is None or inst <= 1e-9:
        notes.append("instructions coefficient is %s: cannot normalise to 1.0, showing raw-scaled values"
                     % ("absent" if inst is None else "%.3f" % inst))
        scale = 1.0
    else:
        scale = 1.0 / inst
    for k in FEATURES:
        if k not in raw:
            notes.append("%s: zero variance in the sample, keeping current %.2f" % (k, cur[k]))
            continue
        c = raw[k] * scale
        if k == "calls":
            # rank.py subtracts WEIGHTS["calls"]*log1p(calls): a negative effect on
            # attempts is the expected sign and maps to a positive weight.
            if c > 0:
                notes.append("calls: fitted effect is +%.2f (more calls = harder); clamping weight to 0" % c)
                w[k] = 0.0
            else:
                w[k] = -c
        else:
            if c < 0:
                notes.append("%s: fitted effect is %.2f (negative difficulty); clamping to 0" % (k, c))
                w[k] = 0.0
            else:
                w[k] = c
    return w, notes


def fmt_weights(w):
    return "{" + ", ".join('"%s": %.2f' % (k, w[k]) for k in
                           ("instructions", "branches", "gte_ops", "stack_size", "jump_tables", "calls")) + "}"


# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--ledger", action="append", help="ledger file(s); default tools/loop/ledger.tsv")
    ap.add_argument("--functions", default=rank.FUNCTIONS)
    ap.add_argument("--giveup-attempts", type=float, default=20.0,
                    help="attempts assigned to a give-up (default 20)")
    ap.add_argument("--lam", type=float, default=1.0, help="ridge penalty on standardised features")
    ap.add_argument("--top", type=int, default=20, help="rows of the old/new top list")
    ap.add_argument("--unit", help="restrict the top list to one unit")
    args = ap.parse_args()

    paths = args.ledger or [ledger.ledger_path()]
    functions = rank.load_functions(args.functions)
    sample, skipped = collect(paths, functions, args.giveup_attempts)
    for key, why in skipped:
        print("skip %s %s: %s" % (key[0], key[1], why))
    n = len(sample)
    if n < 8:
        sys.exit("refit.py: only %d usable outcome(s); need at least 8" % n)

    hist = {}
    for s in sample:
        label = "giveup" if s["status"] == "giveup" else str(int(s["attempts"]))
        hist[label] = hist.get(label, 0) + 1
    print("sample: %d outcomes from %s" % (n, ", ".join(os.path.relpath(p) for p in paths)))
    print("attempts histogram: " + ", ".join("%s x%d" % kv for kv in
          sorted(hist.items(), key=lambda kv: (kv[0] == "giveup", kv[0].rjust(3)))))
    distinct = len(set(s["attempts"] for s in sample))
    if distinct < 3:
        print("WARNING: attempts take only %d distinct value(s); the fit has almost no signal" % distinct)

    attempts = [s["attempts"] for s in sample]
    cur_scores = [rank.score(s["row"]) for s in sample]
    print("spearman(current score, attempts) = %.3f" % spearman(cur_scores, attempts))

    raw, keep, ys, pred, _ = fit(sample, args.lam)
    loo = loo_predictions(sample, args.lam)
    print("spearman(refit, attempts)         = %.3f in-sample, %.3f leave-one-out"
          % (spearman(pred, attempts),
             spearman([p for p in loo if p == p], [a for p, a in zip(loo, attempts) if p == p])))
    print("ridge lambda=%.2f on standardised features; coefficients per unit of feature:" % args.lam)
    for k in FEATURES:
        if k in raw:
            print("  %-13s %+.3f  (log attempts per %s)" % (k, raw[k],
                  "log1p(%s)" % k if k in LOG_FEATURES else k))
        else:
            print("  %-13s  (zero variance in sample)" % k)

    w, notes = propose_weights(raw, keep)
    print()
    print("current  WEIGHTS = " + fmt_weights(rank.WEIGHTS))
    print("proposed WEIGHTS = " + fmt_weights(w))
    for nmsg in notes:
        print("  note: " + nmsg)

    # top-N reorder of the open candidates
    done = {}
    for p in paths:
        done.update(ledger.load(p))
    unit = ledger.norm_unit(args.unit) if args.unit else None
    cur = rank.candidates(functions, done, unit)
    new = []
    for r in cur:
        r2 = dict(r)
        r2["score"] = rank.score(r, w)
        new.append(r2)
    new.sort(key=lambda r: (r["score"], r["size"], r["unit"], r["vaddr"]))
    print()
    print("top %d open candidates%s: current order | proposed order" % (args.top, " in %s" % unit if unit else ""))
    for i in range(min(args.top, len(cur), len(new))):
        a, b = cur[i], new[i]
        print("%3d  %-7s %s %4d %5.2f | %-7s %s %4d %5.2f%s"
              % (i + 1, a["unit"], a["vaddr"], a["size"], a["score"],
                 b["unit"], b["vaddr"], b["size"], b["score"],
                 "" if (a["unit"], a["vaddr"]) == (b["unit"], b["vaddr"]) else "  *"))
    same = len(set((r["unit"], r["vaddr"]) for r in cur[:args.top]) &
               set((r["unit"], r["vaddr"]) for r in new[:args.top]))
    print("%d of the top %d are the same functions under both weightings" % (same, args.top))


if __name__ == "__main__":
    main()
