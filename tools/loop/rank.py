#!/usr/bin/env python3
"""rank.py - order the unmatched functions by estimated matching difficulty.

Reads tools/loop/functions.tsv (inventory.py) and tools/loop/ledger.tsv
(ledger.py), drops PsyQ library functions and everything the ledger marks
matched or given up, and prints the N easiest candidates.

    rank.py                      # top 20 over all units
    rank.py --unit ovl201 -n 50  # one unit
    rank.py --tsv                # machine-readable (unit vaddr name size score ...)
    rank.py --calibrate          # ledger outcomes with their scores, to refit WEIGHTS

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

# hand-weighted difficulty; refit from ledger.tsv when it has enough rows ------
WEIGHTS = {
    "instructions": 1.00,   # log1p: size dominates (more statements to get right)
    "branches":     0.60,   # log1p: control flow (ordering, inverted conditions)
    "gte_ops":      0.80,   # log1p: GTE macros/inline asm are hard to reproduce
    "stack_size":   0.15,   # log1p: locals that live across calls, register pressure
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
    args = ap.parse_args()

    unit = ledger.norm_unit(args.unit) if args.unit else None
    rows = load_functions(args.functions)
    done = ledger.load(args.ledger)

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
