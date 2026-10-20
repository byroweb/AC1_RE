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

Credit-per-effort mode (--bodies): the unit of work is a masked *body*, i.e. a
twins file group (default tools/loop/twins_programs.tsv: exe + ovl201..204), not a
function, because one matched C file is replayed onto every member
(twins.py replay).

    make_queues.py --bodies [-N 6] [--twins F] [--max-size 1000] [--no-tier2]
                   [--limit K] [--prefix queue-S] [--out-dir DIR] [--ledger FILE]

  - open bodies only: no member has a ledger row (match or giveup), via
    rank.mission_candidates(); one row per body, its representative member
    (lowest unit, vaddr), at least --min-instructions long;
  - credit = members per unit of expected effort, payoff = n_members / (1 + score)
    with score = rank.py's difficulty score;
  - tier 1 = gte_ops == 0 and size <= --max-size (1000), sorted by payoff; tier 2
    (GTE or large) follows, also by payoff (--no-tier2 drops it);
  - dealt snake-wise (1..N, N..1, ...) into N disjoint queues, so every queue gets
    the same credit profile and no body is ever in two queues;
  - --limit K keeps the first K rows per queue.

Closeness mode (--bodies --closeness FILE [--min-size 1024]): the bodies the machine
drafts missed, for agents, ordered by how close the best draft already is.  FILE is a
sweep.py report (or tools/loop/closeness.tsv, the same columns without `tries`); bodies
under --min-size bytes are dropped, the rest sorted by closeness x n_members (a body
without a compiling draft counts as closeness 0) and dealt into -N queues.  Rows get
`closeness best_variant` after the usual columns; a body's best draft comes back with
`sweep.py --queue Q --variants <best_variant> --best-dir DIR`.
Rows carry `group n_members payoff tier` in front of the usual columns, so
`drive.py next --queue` and tools/loop/sweep.py read them as they are.
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
BODY_COLS = ["group", "n_members", "payoff", "tier"] + COLS
TWINS_PROGRAMS = os.path.join(HERE, "twins_programs.tsv")


def body_candidates(rows, done, twins_path, max_size=1000, min_instructions=rank.MIN_INSTRUCTIONS,
                    tier2=True):
    """Open bodies, tier 1 then tier 2, each by payoff (members / (1 + score)) descending."""
    groups = rank.load_twins(twins_path)
    cands, _n_done = rank.mission_candidates(rows, done, groups, None, min_instructions)
    for c in cands:
        c["tier"] = 1 if c["gte_ops"] == 0 and c["size"] <= max_size else 2
    cands = [c for c in cands if tier2 or c["tier"] == 1]
    cands.sort(key=lambda c: (c["tier"], -c["payoff"], c["score"], c["unit"], c["vaddr"]))
    return cands


CLOSE_COLS = BODY_COLS + ["closeness", "best_variant"]


def load_closeness(path):
    """{(unit, vaddr lower-case): (closeness or None, best_variant)} from a sweep report."""
    out, hdr = {}, None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            if hdr is None:
                hdr = line.rstrip("\r\n").split("\t")
                continue
            r = dict(zip(hdr, line.rstrip("\r\n").split("\t")))
            c = r.get("closeness", "")
            out[(r["unit"], r["vaddr"].lower())] = (float(c) if c else None, r.get("best_variant", ""))
    return out


def by_closeness(cands, close, min_size):
    """Bodies of at least min_size bytes, by closeness x n_members, then payoff."""
    out = []
    for c in cands:
        if c["size"] < min_size:
            continue
        cl, bv = close.get((c["unit"], str(c["vaddr"]).lower()), (None, ""))
        out.append(dict(c, closeness=cl, best_variant=bv))
    out.sort(key=lambda c: (-(c["closeness"] or 0.0) * c["n_members"], -c["payoff"], c["unit"],
                            str(c["vaddr"])))
    return out


def deal(cands, n):
    """Snake-deal the ranked bodies into n disjoint queues."""
    qs = [[] for _ in range(n)]
    for i, c in enumerate(cands):
        r, k = divmod(i, n)
        qs[k if r % 2 == 0 else n - 1 - k].append(c)
    return qs


def render_bodies(rows, cols=BODY_COLS):
    out = ["\t".join(cols)]
    for r in rows:
        out.append("\t".join(("%.3f" % r[k]) if k in ("score", "payoff", "closeness") and r.get(k) is not None
                             else ("" if r.get(k) is None else str(r[k])) for k in cols))
    return "\n".join(out) + "\n"


def main_bodies(args):
    rows = rank.load_functions(args.functions)
    done = ledger.load(args.ledger)
    cands = body_candidates(rows, done, args.twins, args.max_size, args.min_instructions,
                            not args.no_tier2)
    cols = BODY_COLS
    if args.closeness:
        cands = by_closeness(cands, load_closeness(args.closeness), args.min_size)
        cols = CLOSE_COLS
        have = [c for c in cands if c["closeness"] is not None]
        print("closeness: %d bod(ies) >= %d B, %d with a compiling draft (mean %.3f)"
              % (len(cands), args.min_size, len(have),
                 sum(c["closeness"] for c in have) / len(have) if have else 0.0))
    qs = deal(cands, args.n)
    if args.limit:
        qs = [q[:args.limit] for q in qs]
    names = ["%s%d.tsv" % (args.prefix, i + 1) for i in range(args.n)]
    t1 = sum(1 for c in cands if c["tier"] == 1)
    print("%d open bod(ies) in %s: tier 1 %d, tier 2 %d; %d function(s) credited on a match"
          % (len(cands), os.path.basename(args.twins), t1, len(cands) - t1,
             sum(c["n_members"] for c in cands)))
    for name, q in zip(names, qs):
        print("  %-14s %4d bodies  %5d credited  payoff sum %.1f"
              % (name, len(q), sum(c["n_members"] for c in q), sum(c["payoff"] for c in q)))
    if not args.out_dir:
        print("# dry run: pass --out-dir DIR to write", file=sys.stderr)
        return 0
    out_dir = os.path.abspath(args.out_dir)
    os.makedirs(out_dir, exist_ok=True)
    for name, q in zip(names, qs):
        p = os.path.join(out_dir, name)
        if os.path.normpath(out_dir) == os.path.normpath(HERE) and os.path.exists(p) and not args.force:
            sys.exit("make_queues.py: refusing to overwrite %s (use --force)" % p)
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(render_bodies(q, cols))
        print("wrote %s (%d rows)" % (p, len(q)))
    return 0


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
    g = ap.add_argument_group("credit-per-effort body queues (--bodies)")
    g.add_argument("--bodies", action="store_true", help="rank open twin groups by members / (1 + score)")
    g.add_argument("-N", dest="n", type=int, default=6, help="number of disjoint queues (default 6)")
    g.add_argument("--twins", default=TWINS_PROGRAMS, help="twins file (default tools/loop/twins_programs.tsv)")
    g.add_argument("--max-size", type=int, default=1000, help="tier 1 size limit in bytes (default 1000)")
    g.add_argument("--no-tier2", action="store_true", help="leave out GTE / large bodies")
    g.add_argument("--limit", type=int, default=0, help="rows per queue (default all)")
    g.add_argument("--min-instructions", type=int, default=rank.MIN_INSTRUCTIONS)
    g.add_argument("--prefix", default="queue-S", help="file name prefix (default queue-S -> queue-S1.tsv ...)")
    g.add_argument("--closeness", help="sweep.py report or closeness.tsv: order by closeness x n_members")
    g.add_argument("--min-size", type=int, default=0, help="with --closeness: only bodies of at least this many bytes")
    args = ap.parse_args()
    if args.bodies:
        if args.n < 1:
            sys.exit("make_queues.py: -N wants at least 1")
        return main_bodies(args)

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
