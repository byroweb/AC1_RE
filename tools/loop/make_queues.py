#!/usr/bin/env python3
"""make_queues.py - build the matcher queues (queue-A.tsv / queue-B.tsv) from rank.py.

    make_queues.py [--units ovl201,ovl202] [--out-dir DIR] [--bands 2.5,4.5,6.5]
                   [--picks 14,6,4] [--ledger FILE]

Takes rank.candidates() over the given units (default ovl201,ovl202), sorts by
(score rounded to 3 decimals, size), cuts the list into score bands
(default < 2.5, 2.5-4.5, 4.5-6.5) and from each band takes 2*picks rows evenly
spaced across the band's pool (index round(i*(n-1)/(k-1)), i = 0..k-1), so both
ends of every band are included.  The picks are dealt alternately to A and B
(A gets the even positions), giving two disjoint queues with the same
difficulty profile.  Rows carry the `rank.py --tsv` columns.

Without --out-dir nothing is written: the two queues are printed (dry run).
With --out-dir the files are written there; refuse to overwrite the committed
tools/loop/queue-*.tsv unless --force.  Reproduces the committed queues of
2026-10-18 byte for byte (given the same functions.tsv and ledger).
"""
import argparse
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ledger  # noqa: E402
import rank    # noqa: E402

COLS = ["unit", "vaddr", "name", "size", "instructions", "branches", "calls",
        "jump_tables", "gte_ops", "stack_size", "jt_addrs", "score"]


def spaced(n, k):
    if k <= 0 or n == 0:
        return []
    if k == 1 or n == 1:
        return [0]
    k = min(k, n)
    return [round(i * (n - 1) / (k - 1)) for i in range(k)]


def build(cands, bands, picks):
    pool_all = sorted(cands, key=lambda r: (float("%.3f" % r["score"]), r["size"]))
    seq = []
    lo = 0.0
    for hi, k in zip(bands, picks):
        pool = [r for r in pool_all if lo <= r["score"] < hi]
        idx = spaced(len(pool), 2 * k)
        if len(idx) < 2 * k:
            print("make_queues.py: band %.2f-%.2f has only %d candidate(s) for %d picks"
                  % (lo, hi, len(pool), 2 * k), file=sys.stderr)
        seq += [pool[i] for i in idx]
        lo = hi
    return seq[0::2], seq[1::2]


def render(rows):
    out = ["\t".join(COLS)]
    for r in rows:
        out.append("\t".join(("%.3f" % r[k]) if k == "score" else str(r.get(k, "")) for k in COLS))
    return "\n".join(out) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--units", default="ovl201,ovl202")
    ap.add_argument("--out-dir", help="write queue-A.tsv / queue-B.tsv here (default: print only)")
    ap.add_argument("--bands", default="2.5,4.5,6.5", help="upper score bound of each band")
    ap.add_argument("--picks", default="14,6,4", help="rows per queue from each band")
    ap.add_argument("--ledger", default=None, help="ledger to exclude matched/given-up rows (default tools/loop/ledger.tsv)")
    ap.add_argument("--functions", default=rank.FUNCTIONS)
    ap.add_argument("--force", action="store_true", help="allow overwriting tools/loop/queue-*.tsv")
    args = ap.parse_args()

    bands = [float(x) for x in args.bands.split(",")]
    picks = [int(x) for x in args.picks.split(",")]
    if len(bands) != len(picks):
        sys.exit("make_queues.py: --bands and --picks need the same number of entries")
    units = [ledger.norm_unit(u) for u in args.units.split(",") if u.strip()]
    rows = rank.load_functions(args.functions)
    done = ledger.load(args.ledger)
    cands = [r for r in rank.candidates(rows, done) if r["unit"] in units]
    qa, qb = build(cands, bands, picks)

    if not args.out_dir:
        print("# queue-A.tsv (%d rows)" % len(qa))
        sys.stdout.write(render(qa))
        print("# queue-B.tsv (%d rows)" % len(qb))
        sys.stdout.write(render(qb))
        print("# dry run: pass --out-dir DIR to write", file=sys.stderr)
        return 0
    out_dir = os.path.abspath(args.out_dir)
    if os.path.normpath(out_dir) == os.path.normpath(HERE) and not args.force:
        sys.exit("make_queues.py: refusing to overwrite the committed queues in %s (use --force)" % HERE)
    os.makedirs(out_dir, exist_ok=True)
    for name, q in (("queue-A.tsv", qa), ("queue-B.tsv", qb)):
        p = os.path.join(out_dir, name)
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(render(q))
        print("wrote %s (%d rows)" % (p, len(q)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
