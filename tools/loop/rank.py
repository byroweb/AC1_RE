#!/usr/bin/env python3
"""rank.py - order the unmatched functions by estimated matching difficulty.

Reads tools/loop/functions.tsv (inventory.py) and tools/loop/ledger.tsv
(ledger.py), drops PsyQ library functions and everything the ledger marks
matched or given up, and prints the N easiest candidates.

    rank.py                      # top 20 over all units
    rank.py --unit ovl201 -n 50  # one unit
    rank.py --tsv                # machine-readable (unit vaddr name size score ...)
    rank.py --calibrate          # ledger outcomes with their scores, to refit WEIGHTS
    rank.py --missions           # one open fdat function per twins.tsv group, by payoff

Mission mode (--missions) reads tools/loop/twins.tsv (twins.py group): one
matched C file replays onto every member of its group, so each open group is
listed once (its representative, or its member in --unit) and sorted by

    payoff = n_members / (1 + score)

i.e. manifest rows gained per unit of difficulty.  A group with any ledger row
(match or give-up) on any member is done: the body is the same C everywhere.

Difficulty score (lower = easier):

    score = sum(WEIGHTS[k] * log1p(column k))   for the log1p columns
          + WEIGHTS["jump_tables"] * jump_tables  (each switch table is a flat cost)
          - WEIGHTS["calls"] * log1p(calls)       (jal + argument setup is cheap)

WEIGHTS is a plain dict so it can be refit from the ledger once there are
enough outcomes (attempts per score bucket; see --calibrate).
"""
import argparse
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ledger  # noqa: E402

FUNCTIONS = os.path.join(HERE, "functions.tsv")
TWINS = os.path.join(HERE, "twins.tsv")

# hand-weighted difficulty, last reviewed against 55 ledger rows (refit.py, 2026-10-18)
# Spearman(score, attempts) = 0.63; see README "Weights" for why the fit is not applied literally.
WEIGHTS = {
    "instructions": 1.00,   # log1p: size dominates (more statements to get right)
    "branches":     1.00,   # log1p: control flow; both 2026-10-18 give-ups were branch-heavy (0.60 before refit)
    "gte_ops":      0.80,   # log1p: GTE macros/inline asm are hard to reproduce
    "stack_size":   0.30,   # log1p: frames; every >1-attempt match had one (0.15 before refit)
    "jump_tables":  1.50,   # per table: switch layout + --rodata address needed
    "calls":        0.30,   # log1p, subtracted: calls are cheap instructions
}
MIN_INSTRUCTIONS = 3        # skip `jr ra; nop` empty stubs (--min-instructions 2 to include)


def score(row, w=WEIGHTS):
    s = 0.0
    for k in ("instructions", "branches", "gte_ops", "stack_size"):
        s += w[k] * math.log1p(row[k])
    s += w["jump_tables"] * row["jump_tables"]
    s -= w["calls"] * math.log1p(row["calls"])
    return max(s, 0.0)


def load_functions(path=FUNCTIONS):
    if not os.path.isfile(path):
        sys.exit("rank.py: %s not found; run tools/loop/inventory.py first" % path)
    rows = []
    with open(path, encoding="utf-8") as fh:
        header = fh.readline().rstrip("\n").split("\t")
        ints = {"size", "is_psyq", "instructions", "branches", "calls",
                "jump_tables", "gte_ops", "stack_size"}
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) < len(header):
                continue
            row = dict(zip(header, parts))
            for k in ints:
                row[k] = int(row.get(k) or 0)
            rows.append(row)
    return rows


def candidates(rows, done, unit=None, min_instructions=MIN_INSTRUCTIONS, include_done=False):
    out = []
    for r in rows:
        if r["is_psyq"]:
            continue
        if unit and r["unit"] != unit:
            continue
        if r["instructions"] < min_instructions:
            continue
        if not include_done and (r["unit"], r["vaddr"]) in done:
            continue
        r = dict(r)
        r["score"] = score(r)
        out.append(r)
    out.sort(key=lambda r: (r["score"], r["size"], r["unit"], r["vaddr"]))
    return out


def load_twins(path=TWINS):
    """{group: [(unit, vaddr), ...]} in twins.tsv order (representative first)."""
    if not os.path.isfile(path):
        sys.exit("rank.py: %s not found; run tools/loop/twins.py group first" % path)
    groups = {}
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            p = line.rstrip("\n").split("\t")
            g = groups.setdefault(p[0], [(p[1], p[2])])
            if (p[3], p[4]) != g[0]:
                g.append((p[3], p[4]))
    return groups


def mission_candidates(rows, done, groups, unit=None, min_instructions=MIN_INSTRUCTIONS):
    """One open function per twin group, highest payoff (members per 1 + score) first.

    Returns (candidates, n_done_groups)."""
    by_key = {(r["unit"], r["vaddr"]): r for r in rows}
    out, n_done = [], 0
    for gid, members in groups.items():
        key = members[0]
        if unit:
            pick = [m for m in members if m[0] == unit]
            if not pick:
                continue
            key = pick[0]
        if any(m in done for m in members):
            n_done += 1
            continue
        r = by_key.get(key)
        if r is None or r["is_psyq"] or r["instructions"] < min_instructions:
            continue
        r = dict(r)
        r["score"] = score(r)
        r["group"] = gid
        r["n_members"] = len(members)
        r["payoff"] = len(members) / (1.0 + r["score"])
        out.append(r)
    out.sort(key=lambda r: (-r["payoff"], r["score"], r["unit"], r["vaddr"]))
    return out, n_done


def main_missions(args, unit, rows, done):
    groups = load_twins(args.twins)
    cands, n_done = mission_candidates(rows, done, groups, unit, args.min_instructions)
    if args.max_score is not None:
        cands = [c for c in cands if c["score"] <= args.max_score]
    top = cands[:args.n] if args.n > 0 else cands

    cols = ["group", "n_members", "payoff", "unit", "vaddr", "name", "size", "instructions",
            "branches", "calls", "jump_tables", "gte_ops", "stack_size", "jt_addrs", "score"]
    if args.tsv:
        print("\t".join(cols))
        for c in top:
            c["payoff"] = "%.3f" % c["payoff"]
            c["score"] = "%.3f" % c["score"]
            print("\t".join(str(c[k]) for k in cols))
        return

    print("%d open group(s)%s, %d group(s) already in the ledger; showing %d"
          % (len(cands), " with a member in %s" % unit if unit else "", n_done, len(top)))
    print("%4s %-6s %4s %6s %-7s %-10s %5s %5s %3s %3s %3s %5s %5s"
          % ("#", "group", "mem", "payoff", "unit", "vaddr", "size", "instr", "br", "jal", "jt",
             "stack", "score"))
    for i, c in enumerate(top, 1):
        print("%4d %-6s %4d %6.2f %-7s %-10s %5d %5d %3d %3d %3d %5d %5.2f%s"
              % (i, c["group"], c["n_members"], c["payoff"], c["unit"], c["vaddr"], c["size"],
                 c["instructions"], c["branches"], c["calls"], c["jump_tables"], c["stack_size"],
                 c["score"], ("  rodata " + c["jt_addrs"]) if c["jt_addrs"] else ""))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("-n", type=int, default=20, help="how many candidates (default 20)")
    ap.add_argument("--unit", help="exe, ovl201..ovl204, fdatNNN (default: all)")
    ap.add_argument("--functions", default=FUNCTIONS, help="functions.tsv path")
    ap.add_argument("--ledger", default=None, help="ledger.tsv path (default tools/loop/ledger.tsv)")
    ap.add_argument("--min-instructions", type=int, default=MIN_INSTRUCTIONS)
    ap.add_argument("--max-score", type=float, default=None, help="only candidates at or below this score")
    ap.add_argument("--tsv", action="store_true", help="tab-separated output with a header")
    ap.add_argument("--calibrate", action="store_true",
                    help="list ledger outcomes with their scores (to refit WEIGHTS)")
    ap.add_argument("--missions", action="store_true",
                    help="one open fdat function per twins.tsv group, by members / (1 + score)")
    ap.add_argument("--twins", default=TWINS, help="twins.tsv path (with --missions)")
    args = ap.parse_args()

    unit = ledger.norm_unit(args.unit) if args.unit else None
    rows = load_functions(args.functions)
    done = ledger.load(args.ledger)

    if args.missions:
        main_missions(args, unit, rows, done)
        return

    if args.calibrate:
        by_key = {(r["unit"], r["vaddr"]): r for r in rows}
        print("\t".join(["status", "attempts", "score", "unit", "vaddr", "name", "size",
                         "branches", "calls", "jump_tables", "gte_ops", "stack_size", "reason"]))
        for key, e in sorted(done.items(), key=lambda kv: kv[1]["status"]):
            r = by_key.get(key)
            sc = "%.2f" % score(r) if r else "?"
            vals = [e["status"], e["attempts"], sc, key[0], key[1]]
            if r:
                vals += [r["name"], r["size"], r["branches"], r["calls"], r["jump_tables"],
                         r["gte_ops"], r["stack_size"]]
            else:
                vals += ["(not in functions.tsv)"] + [""] * 6
            vals.append(e.get("reason", ""))
            print("\t".join(str(v) for v in vals))
        return

    cands = candidates(rows, done, unit, args.min_instructions)
    if args.max_score is not None:
        cands = [c for c in cands if c["score"] <= args.max_score]
    top = cands[:args.n] if args.n > 0 else cands

    cols = ["unit", "vaddr", "name", "size", "instructions", "branches", "calls",
            "jump_tables", "gte_ops", "stack_size", "jt_addrs", "score"]
    if args.tsv:
        print("\t".join(cols))
        for c in top:
            c["score"] = "%.3f" % c["score"]
            print("\t".join(str(c[k]) for k in cols))
        return

    skipped = sum(1 for r in rows if not r["is_psyq"] and (not unit or r["unit"] == unit)
                  and (r["unit"], r["vaddr"]) in done)
    print("%d candidate(s)%s, %d already in the ledger; showing %d"
          % (len(cands), " in %s" % unit if unit else "", skipped, len(top)))
    print("%4s %-7s %-10s %-22s %5s %5s %3s %3s %3s %4s %5s %s"
          % ("#", "unit", "vaddr", "name", "size", "instr", "br", "jal", "jt", "gte", "stack", "score"))
    for i, c in enumerate(top, 1):
        print("%4d %-7s %-10s %-22s %5d %5d %3d %3d %3d %4d %5d %5.2f%s"
              % (i, c["unit"], c["vaddr"], c["name"][:22], c["size"], c["instructions"],
                 c["branches"], c["calls"], c["jump_tables"], c["gte_ops"], c["stack_size"],
                 c["score"], ("  rodata " + c["jt_addrs"]) if c["jt_addrs"] else ""))


if __name__ == "__main__":
    main()
