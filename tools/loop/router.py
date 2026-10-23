#!/usr/bin/env python3
"""router.py - send each open function to the cheapest tier likely to close it.

    router.py labels   [--short-max 2] [--out tools/loop/router_labels.tsv]
    router.py features [--report F ...] [--diff F] [--out tools/loop/router_features.tsv]
    router.py eval     [--model rules|tree|gbm|all] [--folds 5] [--holdout 0.2] [--seed 1]
    router.py rank     [--report F ...] [--permute-report F ...] [--near F] [--out F]

Tiers (cheapest first; docs/ROUTER_SCOPE.md):

    1 sweep    tools/loop/sweep.py drafts (m2c + fix-ups) or levers.py rewrites, no LLM
    2 permuter decomp-permuter on the best near-miss draft (permute.py, overnight.sh)
    3 golem    a local LLM's edits on a near miss (golem.py, GOLEM=1)
    4 claude   a Claude session, short: at most --short-max oracle tries
    5 claude   a Claude session, long: more tries than that
    6 open     given up / still open (censored)

`labels` derives the tier that closed every matched function from ledger.tsv's
`reason` (zero-agent sweep / permuter (levers ...) / permuter (decomp-permuter ...) /
golem / empty = Claude), the number of oracle tries from the ledger's `attempts`, the
`matched func_<ADDR> N` commit subjects and the `.attempts` sidecars (the largest of
the three), and marks the rows that were promoted in a batch (the `matched N ...
functions` and `wave-2/3` commits re-verify with one `drive.py try`, so their count
is unknown).  Twin rows (`twin of ...`) are derived matches, not outcomes: they are
written with tier `twin` and never trained on.  `label_source` says where each
label came from; unknowns stay unknown.

`features` writes one row per non-PsyQ function: the functions.tsv columns and
rank.score(), cheap structural ones read from the image words (loops and loop
depth, basic blocks, callee-saved registers, spills, switch vs if-tree, ...), the
twin group and the coarse-skeleton family size (tools/archaeology: the same
source function edited between missions / near-copies across overlays), and the
closest m2c draft's closeness from a sweep report (tools/loop/closeness.tsv by
default: the 2026-10-20 sweep over the then-open bodies; --report adds newer
sweep_report.tsv files, the newest row wins).  --diff F merges a per-function
diff table (unit vaddr words reg_only size_delta) from a compiled draft; nothing
in the repo produces one yet (see "diff hook" below).

`eval` splits the labelled functions by *body* (twins.py groups, so a clone never
sits on both sides), holds out --holdout of them stratified by size bucket, and
also runs --folds grouped cross-validation.  It reports, for the rule baseline
and the learned models, tier accuracy, the confusion matrix, the big-function
bucket on its own, the regret in both directions and the Claude minutes / usage
windows saved against sending everything to Claude; then the projection on the
open set.  Costs are knobs (--window-minutes, --claude-min-per-kb ...), stated
in the output.

`rank` trains on every label and writes the open queue: one row per open
function with the model's tier and probabilities, what the machine tiers have
already tried on it (sweep reports, permute reports, the agents' near misses in
tools/loop/near_misses.tsv) and the tier to send it to next.

Learners: scikit-learn's HistGradientBoostingClassifier / DecisionTreeClassifier
when numpy and scikit-learn import (seconds), else a pure-Python CART tree and a
small gradient-boosted ensemble over binned features (about a minute for eval);
--backend python|sklearn forces one, so the two can be compared.
"""
import argparse
import collections
import csv
import glob
import math
import os
import random
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import ledger  # noqa: E402
import rank    # noqa: E402

LABELS = os.path.join(HERE, "router_labels.tsv")
FEATURES = os.path.join(HERE, "router_features.tsv")
CLOSENESS = os.path.join(HERE, "closeness.tsv")
NEAR = os.path.join(HERE, "near_misses.tsv")
TWINS = os.path.join(HERE, "twins.tsv")
TWINS_PROGRAMS = os.path.join(HERE, "twins_programs.tsv")
LEDGER = os.path.join(HERE, "ledger.tsv")

TIER_NAME = {1: "sweep", 2: "permuter", 3: "golem", 4: "claude-short", 5: "claude-long", 6: "open"}
MACHINE = (1, 2, 3)
SIZE_BUCKETS = (("small", 0, 256), ("mid", 256, 1024), ("big", 1024, 1 << 30))
SREG = set(range(16, 24)) | {30}

# cost model (minutes); every number is a knob of eval / rank
COST = {
    "sweep_cpu": 0.1,          # CPU minutes per body: the sweep is ~25 s per 100 bodies
    "permuter_cpu": 7.0,       # CPU minutes per body: overnight.sh QUICK 120 s + DEEP 300 s
    "golem_gpu": 1.0,          # GPU minutes per body (bake-off: ~40 s)
    "claude_short_per_kb": 3.0,  # Claude minutes per KB, README calibration: 0.5-3 min/KB when it goes well
    "claude_long_per_kb": 8.0,   # 5-10 min/KB for first-of-family bodies with loops / switches
    "claude_short_floor": 2.0,
    "claude_long_floor": 8.0,
    "window_minutes": 720.0,   # one usage window ~ 12 agents for ~1 h (wave 3, batch 1)
}


# ---------------------------------------------------------------------------
# small helpers
# ---------------------------------------------------------------------------
def read_tsv(path, comment="#"):
    rows = []
    with open(path, encoding="utf-8") as fh:
        header = None
        for line in fh:
            line = line.rstrip("\r\n")
            if not line or line.startswith(comment):
                continue
            parts = line.split("\t")
            if header is None:
                header = parts
                continue
            rows.append(dict(zip(header, parts)))
    return rows


def write_tsv(path, rows, cols, comment=None):
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        if comment:
            fh.write("# " + comment + "\n")
        fh.write("\t".join(cols) + "\n")
        for r in rows:
            fh.write("\t".join(fmt(r.get(c, "")) for c in cols) + "\n")


def fmt(v):
    if isinstance(v, float):
        return "%.3f" % v
    return str(v)


def norm_vaddr(v):
    return "0x%08X" % int(v, 16)


def size_bucket(size):
    for name, lo, hi in SIZE_BUCKETS:
        if lo <= size < hi:
            return name
    return SIZE_BUCKETS[-1][0]


def load_groups(paths=(TWINS_PROGRAMS, TWINS)):
    """{(unit, vaddr): (group, n_members)} over both twins files."""
    out = {}
    for p in paths:
        if not os.path.isfile(p):
            continue
        with open(p, encoding="utf-8") as fh:
            for line in fh:
                if line.startswith("#") or not line.strip():
                    continue
                q = line.rstrip("\n").split("\t")
                out[(q[3], norm_vaddr(q[4]))] = (q[0], int(q[6]))
    return out


def load_report(path):
    """{(unit, vaddr): row} of a sweep report / closeness.tsv (status, closeness, positional, best_variant)."""
    out = {}
    for r in read_tsv(path):
        if not r.get("unit"):
            continue
        out[(r["unit"], norm_vaddr(r["vaddr"]))] = r
    return out


def expand_by_group(report, groups):
    """A sweep report has one row per body (its representative); its twins share the draft,
    so give every member of the group the representative's row (members' own rows win)."""
    members = collections.defaultdict(list)
    for key, (g, _n) in groups.items():
        members[g].append(key)
    out = dict(report)
    for key, row in report.items():
        g = groups.get(key)
        if g is None:
            continue
        for m in members[g[0]]:
            if m not in report:
                out[m] = dict(row, _twin_of=key)
    return out


def claude_minutes(size, tier, cost=COST):
    kb = size / 1024.0
    if tier == 4:
        return max(cost["claude_short_floor"], cost["claude_short_per_kb"] * kb)
    if tier == 5:
        return max(cost["claude_long_floor"], cost["claude_long_per_kb"] * kb)
    # unknown length: the mean of the two
    return 0.5 * (claude_minutes(size, 4, cost) + claude_minutes(size, 5, cost))


# ---------------------------------------------------------------------------
# labels
# ---------------------------------------------------------------------------
RE_MATCHED_N = re.compile(r"^matched func_([0-9A-Fa-f]{8}) (\d+)$")
RE_BATCH = re.compile(r"^matched \d+ ")


def ledger_commits(ledger_path):
    """{raw ledger line: (short hash, subject)} of the commit that first added each line."""
    rel = os.path.relpath(ledger_path, REPO)
    try:
        out = subprocess.run(["git", "log", "-p", "--format=COMMIT%x09%h%x09%s", "--", rel],
                             cwd=REPO, capture_output=True, text=True, encoding="utf-8",
                             errors="replace").stdout
    except OSError:
        return {}
    cur, first = None, {}
    for line in out.splitlines():
        if line.startswith("COMMIT\t"):
            cur = tuple(line.split("\t")[1:3])
        elif line.startswith("+") and not line.startswith("+++") and cur:
            first[line[1:].rstrip("\r")] = cur      # git log is newest first: the last write wins = oldest
    return first


def sidecar_tries(unit, vaddr):
    p = os.path.join(REPO, "re", "match", unit, "func_%s.attempts" % vaddr[2:])
    if not os.path.isfile(p):
        return None
    with open(p, encoding="utf-8") as fh:
        return sum(1 for line in fh if line.strip())


def classify_reason(reason):
    """-> (tier or None, closer) from a ledger reason."""
    r = reason.strip()
    if r.startswith("twin of"):
        return None, "twin"
    if r.startswith("zero-agent sweep"):
        return 1, "sweep"
    if r.startswith("permuter ("):
        return (2, "permuter") if "decomp-permuter" in r else (1, "levers")
    if r.startswith("golem ("):
        return 3, "golem"
    if r == "":
        return None, "claude"
    return None, "other"


def build_labels(ledger_path, short_max, functions):
    """One row per ledger key (newest row wins, as ledger.load does)."""
    by_key = {(r["unit"], r["vaddr"]): r for r in functions}
    groups = load_groups()
    commits = ledger_commits(ledger_path)
    raw = {}
    with open(ledger_path, encoding="utf-8") as fh:
        header = None
        for line in fh:
            s = line.rstrip("\r\n")
            if not s or s.startswith("#"):
                continue
            p = s.split("\t")
            if header is None:
                header = p
                continue
            row = dict(zip(header, p))
            raw[(row["unit"], norm_vaddr(row["vaddr"]))] = (row, s)
    rows = []
    for key in sorted(raw, key=lambda k: (k[0] != "exe", k[0], k[1])):
        row, line = raw[key]
        unit, vaddr = key
        f = by_key.get(key)
        commit = commits.get(line, ("", ""))
        subj = commit[1]
        try:
            att = int(row.get("attempts") or 0)
        except ValueError:
            att = 0
        src = ["ledger:%s" % (row["status"])]
        tier, closer = classify_reason(row.get("reason", ""))
        tries, known = att, att > 0
        if closer == "claude":
            m = RE_MATCHED_N.match(subj)
            if m and m.group(1).upper() == vaddr[2:]:
                n = int(m.group(2))
                if n > tries:
                    tries, known = n, True
                src.append("commit:matched-N")
            elif "wave-" in subj or RE_BATCH.match(subj):
                # promoted from an agent's scratch dir: one re-verifying drive.py try
                if tries <= 1:
                    known = False
                src.append("commit:batch-promote")
            elif subj:
                src.append("commit:%s" % ("seed" if "seeded" in subj else "other"))
            sc = sidecar_tries(unit, vaddr)
            if sc:
                src.append("sidecar")
                if sc > tries:
                    tries, known = sc, True
            if row["status"] == "giveup":
                tier = 6
            elif known:
                tier = 4 if tries <= short_max else 5
            else:
                tier = "4/5"
        elif closer == "twin":
            tier = "twin"
            src.append("derived")
        elif closer == "other":
            tier = 6 if row["status"] == "giveup" else "?"
        if row["status"] == "giveup":
            tier = 6
        rows.append({
            "unit": unit, "vaddr": vaddr, "size": f["size"] if f else "",
            "group": groups.get(key, ("", 1))[0], "n_members": groups.get(key, ("", 1))[1],
            "tier": tier, "closer": closer, "tries": tries if known else "",
            "tries_known": int(known), "status": row["status"], "when": row.get("when", ""),
            "commit": commit[0], "label_source": "+".join(src),
            "reason": row.get("reason", "").replace("\t", " ")[:120],
        })
    return rows


def main_labels(args):
    functions = rank.load_functions(args.functions)
    rows = build_labels(args.ledger, args.short_max, functions)
    cols = ["unit", "vaddr", "size", "group", "n_members", "tier", "closer", "tries", "tries_known",
            "status", "when", "commit", "label_source", "reason"]
    write_tsv(args.out, rows, cols,
              "tier that closed each ledger row (router.py labels, short-max %d): 1 sweep/levers, 2 decomp-permuter, "
              "3 golem, 4 Claude <= %d tries, 5 Claude > %d tries, 4/5 Claude with an unknown try count, 6 give-up, "
              "twin = derived from another body's C (never trained on)" % (args.short_max, args.short_max, args.short_max))
    n = collections.Counter(str(r["tier"]) for r in rows)
    print("wrote %s: %d rows" % (os.path.relpath(args.out, REPO), len(rows)))
    for t in ("1", "2", "3", "4", "5", "4/5", "6", "twin", "?"):
        if n[t]:
            print("  tier %-4s %5d  %s" % (t, n[t], {
                "1": "sweep + levers (no LLM)", "2": "decomp-permuter", "3": "golem",
                "4": "Claude, <= %d tries" % args.short_max, "5": "Claude, > %d tries" % args.short_max,
                "4/5": "Claude, try count unknown (batch promotes, seeds)", "6": "give-up",
                "twin": "derived (excluded)", "?": "unclassified reason"}[t]))
    cl = collections.Counter(r["closer"] for r in rows if r["closer"] != "twin")
    print("  closer: " + ", ".join("%s %d" % kv for kv in sorted(cl.items())))
    src = collections.Counter(r["label_source"] for r in rows if r["closer"] == "claude")
    print("  Claude label sources: " + ", ".join("%s %d" % kv for kv in src.most_common()))
    return 0


# ---------------------------------------------------------------------------
# features
# ---------------------------------------------------------------------------
STATIC_COLS = ["size", "instructions", "branches", "calls", "jump_tables", "gte_ops", "stack_size", "score",
               "n_members", "family_size", "family_units", "family_extra",
               "is_exe", "is_ovl201", "is_ovl202", "is_ovl203", "is_ovl204", "is_fdat"]
WORD_COLS = ["loops", "loop_depth", "loop_span", "blocks", "max_block", "cond_br", "uncond", "jr_tab", "jalr",
             "returns", "sregs", "s8", "frame", "sw_sp", "muldiv", "div", "lbh", "sext", "slt", "unal",
             "lui", "loads", "stores", "gp", "psyq_calls", "ncallees", "nops", "gte_pairs"]
DRAFT_COLS = ["has_draft", "closeness", "positional", "shift", "diff_words", "variant_class"]
DIFF_COLS = ["diff_words_real", "reg_only", "size_delta"]
FEATURE_COLS = STATIC_COLS + WORD_COLS + DRAFT_COLS + DIFF_COLS
VARIANT_CLASS = [("neighbour", 3), ("ctx-fix", 1), ("ctx-spframe", 2), ("ctx-spdecl", 2), ("ctx", 0), ("file", 4)]


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def coarse_key(words):
    out = []
    for w in words:
        op = w >> 26
        out.append(w & 0xFC000000 if op in (2, 3) else w if op == 0 else w & 0xFFFF0000)
    return hash(tuple(out))


def word_features(words, va0, psyq):
    n = len(words)
    f = dict((k, 0) for k in WORD_COLS)
    leaders = {0}
    loops = []
    callees = set()
    gte_last = -9
    for i, w in enumerate(words):
        op, rs, rt, fn, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 63, s16(w & 0xFFFF)
        if w == 0:
            f["nops"] += 1
        if op == 3:
            t = ((va0 + 4 * i) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
            callees.add(t)
            if t in psyq:
                f["psyq_calls"] += 1
            leaders.add(i + 2)
        elif op == 2:
            t = ((va0 + 4 * i) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
            k = (t - va0) // 4
            if 0 <= k < n:
                leaders.add(k)
                if k <= i:
                    loops.append((k, i))
            f["uncond"] += 1
            leaders.add(i + 2)
        elif op == 0:
            if fn == 8:
                if rs == 31:
                    f["returns"] += 1
                else:
                    f["jr_tab"] += 1
                leaders.add(i + 2)
            elif fn == 9:
                f["jalr"] += 1
                leaders.add(i + 2)
            elif fn in (0x18, 0x19, 0x1A, 0x1B):
                f["muldiv"] += 1
                if fn in (0x1A, 0x1B):
                    f["div"] += 1
            elif fn in (0x2A, 0x2B):
                f["slt"] += 1
            elif fn == 3 and ((w >> 6) & 31) in (16, 24):
                f["sext"] += 1
        elif op in (4, 5, 6, 7, 1):
            k = i + 1 + imm
            if 0 <= k < n:
                leaders.add(k)
            leaders.add(i + 2)
            if op == 4 and rs == 0 and rt == 0:
                f["uncond"] += 1
            else:
                f["cond_br"] += 1
            if imm < 0:
                f["loops"] += 1
                loops.append((max(k, 0), i))
        elif op in (0x22, 0x26, 0x2A, 0x2E):
            f["unal"] += 1
        elif op in (0x12, 0x32, 0x3A):
            if i - gte_last <= 2:
                f["gte_pairs"] += 1
            gte_last = i
        elif op == 0x0F:
            f["lui"] += 1
        elif op in (0x0A, 0x0B):
            f["slt"] += 1
        if op == 9 and rs == 29 and rt == 29 and imm < 0 and i < 4:
            f["frame"] = -imm
        if op in (0x20, 0x21, 0x24, 0x25):
            f["lbh"] += 1
        if op in (0x20, 0x21, 0x23, 0x24, 0x25):
            f["loads"] += 1
        if op in (0x28, 0x29, 0x2B):
            f["stores"] += 1
        if op in (0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 9) and rs == 28:
            f["gp"] += 1
        if op == 0x2B and rs == 29 and i < 16 and rt in SREG:
            f["sregs"] += 1
            if rt == 30:
                f["s8"] = 1
        if op in (0x23, 0x2B) and rs == 29 and 16 <= i < n - 14:
            f["sw_sp"] += 1
    f["ncallees"] = len(callees)
    lead = sorted(k for k in leaders if 0 <= k < n)
    f["blocks"] = len(lead)
    f["max_block"] = max((b - a for a, b in zip(lead, lead[1:] + [n])), default=n)
    if loops:
        depth = [0] * n
        for a, b in loops:
            for k in range(a, b + 1):
                depth[k] += 1
        f["loop_depth"] = max(depth)
        f["loop_span"] = sum(1 for d in depth if d > 0)
    return f


def draft_features(key, reports):
    """closeness columns from the newest report row of the function (or its group representative)."""
    out = dict(has_draft=0, closeness=-1.0, positional=-1.0, shift=-1.0, diff_words=-1, variant_class=-1)
    r = None
    for rep in reports:                       # oldest first; the newest wins
        if key in rep:
            r = rep[key]
    if r is None:
        return out, ""
    try:
        c = float(r.get("closeness") or "")
    except ValueError:
        return out, r.get("_src", "")
    try:
        p = float(r.get("positional") or c)
    except ValueError:
        p = c
    out.update(has_draft=1, closeness=c, positional=p, shift=c - p, variant_class=-1)
    bv = r.get("best_variant", "")
    for name, cls in VARIANT_CLASS:
        if bv.startswith(name):
            out["variant_class"] = cls
            break
    return out, r.get("_src", "")


def build_features(functions, reports, diff_rows=None, need_words=True):
    import twins  # noqa: E402  (reads the images; only here)
    psyq = {int(r["vaddr"], 16) for r in functions if r["is_psyq"]}
    groups = load_groups()
    rows, words_of = [], {}
    missing = 0
    for r in functions:
        if r["is_psyq"]:
            continue
        key = (r["unit"], r["vaddr"])
        va = int(r["vaddr"], 16)
        words = None
        if need_words:
            try:
                words = twins.func_words(r["unit"], va, r["size"])
            except Exception:
                missing += 1
        words_of[key] = words
        row = {"unit": r["unit"], "vaddr": r["vaddr"], "name": r["name"], "size": r["size"],
               "instructions": r["instructions"], "branches": r["branches"], "calls": r["calls"],
               "jump_tables": r["jump_tables"], "gte_ops": r["gte_ops"], "stack_size": r["stack_size"],
               "score": rank.score(r)}
        g = groups.get(key, ("", 1))
        row["group"], row["n_members"] = g[0] or ("%s:%s" % key), g[1]
        for u in ("exe", "ovl201", "ovl202", "ovl203", "ovl204"):
            row["is_" + u] = int(r["unit"] == u)
        row["is_fdat"] = int(r["unit"].startswith("fdat"))
        if words is not None:
            row.update(word_features(words, va, psyq))
            row["_coarse"] = coarse_key(words)
        else:
            row.update((k, -1) for k in WORD_COLS)
            row["_coarse"] = None
        d, src = draft_features(key, reports)
        row.update(d)
        row["closeness_src"] = src
        if d["has_draft"]:
            row["diff_words"] = int(round((1.0 - d["closeness"]) * r["instructions"]))
        row.update(diff_words_real=-1, reg_only=-1, size_delta=0)
        if diff_rows and key in diff_rows:
            row.update(diff_rows[key])
        rows.append(row)
    fam = collections.Counter(r["_coarse"] for r in rows if r["_coarse"] is not None)
    fam_units = collections.defaultdict(set)
    for r in rows:
        if r["_coarse"] is not None:
            fam_units[r["_coarse"]].add(r["unit"])
    for r in rows:
        c = r.pop("_coarse")
        r["family_size"] = fam[c] if c is not None else 1
        r["family_units"] = len(fam_units[c]) if c is not None else 1
        r["family_extra"] = max(r["family_size"] - r["n_members"], 0)
    return rows, missing


def load_reports(paths, groups=None):
    reps = []
    groups = groups if groups is not None else load_groups()
    for p in paths:
        rep = load_report(p)
        src = os.path.basename(os.path.dirname(p)) + "/" + os.path.basename(p) if "sweep_report" in p \
            else os.path.basename(p)
        for v in rep.values():
            v["_src"] = src
        reps.append(expand_by_group(rep, groups))
    return reps


def load_diff(path):
    out = {}
    for r in read_tsv(path):
        out[(r["unit"], norm_vaddr(r["vaddr"]))] = {
            "diff_words_real": int(r.get("words") or -1), "reg_only": int(r.get("reg_only") or -1),
            "size_delta": int(r.get("size_delta") or 0)}
    return out


def main_features(args):
    functions = rank.load_functions(args.functions)
    reports = load_reports([CLOSENESS] + list(args.report or []))
    diff = load_diff(args.diff) if args.diff else None
    rows, missing = build_features(functions, reports, diff)
    cols = ["unit", "vaddr", "name", "group", "closeness_src"] + FEATURE_COLS
    write_tsv(args.out, rows, cols,
              "router.py features: static + image-derived + draft-closeness features per non-PsyQ function "
              "(-1 = not available)")
    print("wrote %s: %d rows (%d without image words, %d with a draft closeness)"
          % (os.path.relpath(args.out, REPO), len(rows), missing, sum(r["has_draft"] for r in rows)))
    return 0


# ---------------------------------------------------------------------------
# learners (pure Python, binned features)
# ---------------------------------------------------------------------------
class Binner:
    """Per-feature thresholds at up to `nbins` quantiles; values map to bin indices."""

    def __init__(self, nbins=32):
        self.nbins = nbins
        self.edges = []

    def fit(self, X):
        nf = len(X[0])
        self.edges = []
        for f in range(nf):
            vals = sorted(set(row[f] for row in X))
            if len(vals) <= self.nbins:
                edges = vals[:-1]
            else:
                edges = sorted(set(vals[int(len(vals) * (i + 1) / self.nbins) - 1] for i in range(self.nbins - 1)))
            self.edges.append(edges)
        return self

    def transform(self, X):
        """-> column-major bins: bins[f][i]."""
        cols = []
        for f, edges in enumerate(self.edges):
            col = []
            for row in X:
                v = row[f]
                lo, hi = 0, len(edges)
                while lo < hi:            # first edge >= v  -> bin index
                    mid = (lo + hi) // 2
                    if edges[mid] < v:
                        lo = mid + 1
                    else:
                        hi = mid
                col.append(lo)
            cols.append(col)
        return cols

    def threshold(self, f, b):
        return self.edges[f][b]


def _split_rows(bins, f, b, rows):
    col = bins[f]
    left = [i for i in rows if col[i] <= b]
    right = [i for i in rows if col[i] > b]
    return left, right


def fit_reg_tree(bins, nbins, G, H, rows, depth, max_depth, min_leaf, lam):
    """Newton regression tree on gradients (leaf value -G/(H+lam)); -> nested tuple / float."""
    Gt = sum(G[i] for i in rows)
    Ht = sum(H[i] for i in rows)
    leaf = -Gt / (Ht + lam)
    if depth >= max_depth or len(rows) < 2 * min_leaf:
        return leaf
    base = Gt * Gt / (Ht + lam)
    best = None
    for f in range(len(bins)):
        nb = nbins[f]
        if nb < 2:
            continue
        gb, hb, cb = [0.0] * nb, [0.0] * nb, [0] * nb
        col = bins[f]
        for i in rows:
            b = col[i]
            gb[b] += G[i]
            hb[b] += H[i]
            cb[b] += 1
        gl = hl = 0.0
        cl = 0
        for b in range(nb - 1):
            gl += gb[b]
            hl += hb[b]
            cl += cb[b]
            if cl < min_leaf or len(rows) - cl < min_leaf:
                continue
            gain = gl * gl / (hl + lam) + (Gt - gl) ** 2 / (Ht - hl + lam) - base
            if best is None or gain > best[0]:
                best = (gain, f, b)
    if best is None or best[0] <= 1e-6:
        return leaf
    _, f, b = best
    left, right = _split_rows(bins, f, b, rows)
    return (f, b, fit_reg_tree(bins, nbins, G, H, left, depth + 1, max_depth, min_leaf, lam),
            fit_reg_tree(bins, nbins, G, H, right, depth + 1, max_depth, min_leaf, lam))


def fit_class_tree(bins, nbins, y, w, rows, K, depth, max_depth, min_leaf):
    """CART (weighted gini); leaves hold class probabilities."""
    tot = [0.0] * K
    for i in rows:
        tot[y[i]] += w[i]
    s = sum(tot) or 1.0
    leaf = [t / s for t in tot]
    if depth >= max_depth or len(rows) < 2 * min_leaf or max(leaf) >= 0.999:
        return leaf

    def gini(cnt):
        n = sum(cnt)
        return n * (1.0 - sum((c / n) ** 2 for c in cnt)) if n > 0 else 0.0
    base = gini(tot)
    best = None
    for f in range(len(bins)):
        nb = nbins[f]
        if nb < 2:
            continue
        cb = [[0.0] * K for _ in range(nb)]
        nb_rows = [0] * nb
        col = bins[f]
        for i in rows:
            cb[col[i]][y[i]] += w[i]
            nb_rows[col[i]] += 1
        left = [0.0] * K
        cl = 0
        for b in range(nb - 1):
            for k in range(K):
                left[k] += cb[b][k]
            cl += nb_rows[b]
            if cl < min_leaf or len(rows) - cl < min_leaf:
                continue
            right = [tot[k] - left[k] for k in range(K)]
            gain = base - gini(left) - gini(right)
            if best is None or gain > best[0]:
                best = (gain, f, b)
    if best is None or best[0] <= 1e-9:
        return leaf
    _, f, b = best
    left, right = _split_rows(bins, f, b, rows)
    return (f, b, fit_class_tree(bins, nbins, y, w, left, K, depth + 1, max_depth, min_leaf),
            fit_class_tree(bins, nbins, y, w, right, K, depth + 1, max_depth, min_leaf))


def tree_predict(tree, binrow):
    while isinstance(tree, tuple):
        f, b, l, r = tree
        tree = l if binrow[f] <= b else r
    return tree


def tree_depth_paths(tree, binner, names, out, path=""):
    """Human-readable rules of a classification tree."""
    if not isinstance(tree, tuple):
        out.append("%s -> %s" % (path or "(root)", " ".join("%.2f" % p for p in tree)))
        return
    f, b, l, r = tree
    thr = binner.threshold(f, b)
    tree_depth_paths(l, binner, names, out, path + (" & " if path else "") + "%s <= %g" % (names[f], thr))
    tree_depth_paths(r, binner, names, out, path + (" & " if path else "") + "%s > %g" % (names[f], thr))


class GBM:
    """Multiclass gradient boosting (softmax, Newton leaves) over binned features."""

    def __init__(self, K, rounds=80, depth=3, lr=0.1, min_leaf=8, lam=1.0, class_weight=None, nbins=32):
        self.K, self.rounds, self.depth, self.lr, self.min_leaf, self.lam = K, rounds, depth, lr, min_leaf, lam
        self.class_weight = class_weight or [1.0] * K
        self.nbins = nbins
        self.trees = []
        self.binner = None
        self.prior = None

    def fit(self, X, y):
        n = len(X)
        self.binner = Binner(self.nbins).fit(X)
        bins = self.binner.transform(X)
        nbins = [len(e) + 1 for e in self.binner.edges]
        cnt = [max(sum(1 for v in y if v == k), 1) for k in range(self.K)]
        self.prior = [math.log(c / n) for c in cnt]
        F = [list(self.prior) for _ in range(n)]
        rows = list(range(n))
        for _ in range(self.rounds):
            round_trees = []
            for k in range(self.K):
                G, H = [0.0] * n, [0.0] * n
                for i in range(n):
                    m = max(F[i])
                    ex = [math.exp(v - m) for v in F[i]]
                    p = ex[k] / sum(ex)
                    wgt = self.class_weight[y[i]]
                    G[i] = (p - (1.0 if y[i] == k else 0.0)) * wgt
                    H[i] = max(p * (1.0 - p), 1e-6) * wgt
                t = fit_reg_tree(bins, nbins, G, H, rows, 0, self.depth, self.min_leaf, self.lam)
                round_trees.append(t)
            for k, t in enumerate(round_trees):
                for i in range(n):
                    F[i][k] += self.lr * tree_predict(t, [bins[f][i] for f in range(len(bins))])
            self.trees.append(round_trees)
        return self

    def predict_proba(self, X):
        bins = self.binner.transform(X)
        out = []
        for i in range(len(X)):
            br = [bins[f][i] for f in range(len(bins))]
            F = list(self.prior)
            for round_trees in self.trees:
                for k, t in enumerate(round_trees):
                    F[k] += self.lr * tree_predict(t, br)
            m = max(F)
            ex = [math.exp(v - m) for v in F]
            s = sum(ex)
            out.append([e / s for e in ex])
        return out


class Tree:
    """A single CART classification tree (interpretable baseline)."""

    def __init__(self, K, depth=5, min_leaf=12, class_weight=None, nbins=32):
        self.K, self.depth, self.min_leaf, self.nbins = K, depth, min_leaf, nbins
        self.class_weight = class_weight or [1.0] * K

    def fit(self, X, y):
        self.binner = Binner(self.nbins).fit(X)
        bins = self.binner.transform(X)
        nbins = [len(e) + 1 for e in self.binner.edges]
        w = [self.class_weight[v] for v in y]
        self.tree = fit_class_tree(bins, nbins, y, w, list(range(len(X))), self.K, 0, self.depth, self.min_leaf)
        return self

    def predict_proba(self, X):
        bins = self.binner.transform(X)
        return [list(tree_predict(self.tree, [bins[f][i] for f in range(len(bins))])) for i in range(len(X))]

    def rules(self, names):
        out = []
        tree_depth_paths(self.tree, self.binner, names, out)
        return out


# ---------------------------------------------------------------------------
# rule baseline
# ---------------------------------------------------------------------------
class Rules:
    """Hand-shaped rules with thresholds fitted on the training rows (grid search on accuracy).

    machine if instructions <= I and no jump table [static], or the best draft is within
    C of the retail words [post-sweep]; among Claude rows, short if rank score <= S."""

    GRID_I = (16, 24, 32, 40, 48, 64, 80, 100, 140)
    GRID_C = (0.90, 0.95, 0.97, 0.98, 0.99, 1.01)
    GRID_S = (3.0, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 8.0)

    def __init__(self, K, names, use_draft):
        self.K, self.names, self.use_draft = K, names, use_draft
        self.ix = {n: i for i, n in enumerate(names)}

    def _machine(self, row, I, C):
        if row[self.ix["instructions"]] <= I and row[self.ix["jump_tables"]] == 0:
            return True
        if self.use_draft and row[self.ix["has_draft"]] and row[self.ix["closeness"]] >= C:
            return True
        return False

    def _pred(self, row, I, C, S):
        if self._machine(row, I, C):
            return 0
        if self.K == 2:
            return 1
        return 1 if row[self.ix["score"]] <= S else 2

    def fit(self, X, y):
        best = None
        for I in self.GRID_I:
            for C in (self.GRID_C if self.use_draft else (1.01,)):
                for S in (self.GRID_S if self.K == 3 else (0.0,)):
                    acc = sum(1 for row, v in zip(X, y) if self._pred(row, I, C, S) == v)
                    if best is None or acc > best[0]:
                        best = (acc, I, C, S)
        self.I, self.C, self.S = best[1:]
        return self

    def predict_proba(self, X):
        out = []
        for row in X:
            p = [0.0] * self.K
            p[self._pred(row, self.I, self.C, self.S)] = 1.0
            out.append(p)
        return out

    def describe(self):
        s = "machine if instructions <= %d and no jump table" % self.I
        if self.use_draft and self.C <= 1.0:
            s += ", or draft closeness >= %.2f" % self.C
        if self.K == 3:
            s += "; Claude short if score <= %.1f" % self.S
        return s


# ---------------------------------------------------------------------------
# evaluation
# ---------------------------------------------------------------------------
def load_dataset(labels_path, features_path):
    feats = {(r["unit"], r["vaddr"]): r for r in read_tsv(features_path)}
    rows = []
    for l in read_tsv(labels_path):
        f = feats.get((l["unit"], l["vaddr"]))
        if f is None or l["tier"] in ("twin", "?"):
            continue
        r = dict(f)
        for k in FEATURE_COLS:
            r[k] = float(r.get(k, -1) or -1)
        r["tier"] = l["tier"]
        r["closer"] = l["closer"]
        r["when"] = l.get("when", "")
        r["tries"] = int(l["tries"]) if l["tries"] else None
        r["size"] = float(f["size"])
        rows.append(r)
    return rows


def group_split(rows, holdout, seed, folds=None):
    """Assign every row a fold id by twin group, stratified by size bucket.
    holdout: fraction (fold 0 = test); folds: k-fold ids instead."""
    rnd = random.Random(seed)
    by_bucket = collections.defaultdict(dict)
    for r in rows:
        by_bucket[size_bucket(r["size"])].setdefault(r["group"], []).append(r)
    fold_of = {}
    for bucket, groups in by_bucket.items():
        gids = list(groups)
        rnd.shuffle(gids)
        if folds:
            for i, g in enumerate(gids):
                fold_of[g] = i % folds
        else:
            n = sum(len(groups[g]) for g in gids)
            acc = 0
            for g in gids:
                fold_of[g] = 0 if acc < holdout * n else 1
                acc += len(groups[g])
    return fold_of


def task_rows(rows, task, claude_since=""):
    """-> (rows, y, K, class names) for a task.

    static3 : machine (1-3) / Claude short (4) / Claude long (5, 6); Claude rows with an unknown
              try count are left out.  Static features only.
    static2 : machine / Claude (all Claude rows, known or not, plus give-ups).  Static only.
    draft2  : rows the first sweep missed (a draft closeness exists): machine (levers,
              permuter, golem) / Claude.  Static + draft features.
    draft3  : same rows, machine / Claude short / Claude long (known try counts).

    claude_since: Claude rows matched before this date never had the sweep tried on them
    (it did not exist), so their tier-1 outcome is unknown; the static tasks leave them out.
    """
    out, y = [], []
    for r in rows:
        t = r["tier"]
        if task.startswith("draft") and not r["has_draft"]:
            continue
        if task.startswith("draft") and r["closer"] == "sweep":
            continue          # the sweep closed it before any draft was reported
        if task.startswith("static") and claude_since and t not in ("1", "2", "3") and r["when"] < claude_since:
            continue
        if t in ("1", "2", "3"):
            cls = 0
        elif t == "4":
            cls = 1
        elif t in ("5", "6"):
            cls = 2
        elif t == "4/5":
            if task.endswith("3"):
                continue
            cls = 1
        else:
            continue
        if task.endswith("2") and cls == 2:
            cls = 1
        out.append(r)
        y.append(cls)
    K = 3 if task.endswith("3") else 2
    names = ["machine", "claude-short", "claude-long"][:K] if K == 3 else ["machine", "claude"]
    return out, y, K, names


def feature_names(task):
    cols = STATIC_COLS + WORD_COLS
    if task.startswith("draft"):
        cols = cols + DRAFT_COLS + DIFF_COLS
    return cols


def matrix(rows, cols):
    return [[r[c] for c in cols] for r in rows]


class SkModel:
    """scikit-learn backend with the same fit / predict_proba / rules interface as GBM and Tree.
    class_weight (a per-class list, set before fit) turns into class_weight="balanced"."""

    def __init__(self, kind, K, args):
        self.kind, self.K, self.args = kind, K, args
        self.class_weight = None
        self.model = None

    def _balanced(self):
        return self.class_weight is not None and any(abs(w - 1.0) > 1e-9 for w in self.class_weight)

    def fit(self, X, y):
        import numpy as np
        from sklearn.ensemble import HistGradientBoostingClassifier
        from sklearn.tree import DecisionTreeClassifier
        cw = "balanced" if self._balanced() else None
        a = self.args
        if self.kind == "tree":
            self.model = DecisionTreeClassifier(max_depth=a.tree_depth, min_samples_leaf=a.min_leaf,
                                                class_weight=cw, random_state=0)
        else:
            self.model = HistGradientBoostingClassifier(max_iter=a.rounds, max_depth=a.depth, learning_rate=a.lr,
                                                        min_samples_leaf=a.min_leaf, class_weight=cw,
                                                        random_state=0)
        self.model.fit(np.asarray(X, dtype=float), np.asarray(y))
        return self

    def predict_proba(self, X):
        import numpy as np
        p = self.model.predict_proba(np.asarray(X, dtype=float))
        # classes absent from the training rows get probability 0
        out = []
        cls = list(self.model.classes_)
        for row in p:
            full = [0.0] * self.K
            for c, v in zip(cls, row):
                full[int(c)] = float(v)
            out.append(full)
        return out

    def rules(self, names):
        from sklearn.tree import export_text
        return export_text(self.model, feature_names=list(names), max_depth=self.args.tree_depth).splitlines()


def sklearn_available():
    try:
        import numpy  # noqa: F401
        import sklearn  # noqa: F401
        return True
    except ImportError:
        return False


def backend_name(args):
    """'python' or 'sklearn' for --backend auto|python|sklearn (auto = sklearn when it imports)."""
    b = getattr(args, "backend", "auto")
    if b == "auto":
        return "sklearn" if sklearn_available() else "python"
    if b == "sklearn" and not sklearn_available():
        sys.exit("router.py: --backend sklearn but numpy / scikit-learn do not import")
    return b


def make_model(kind, K, names, task, args):
    if kind == "rules":
        return Rules(K, names, task.startswith("draft"))
    if backend_name(args) == "sklearn":
        return SkModel(kind, K, args)
    cw = None
    if args.balance:
        cw = [1.0] * K
    if kind == "tree":
        return Tree(K, depth=args.tree_depth, min_leaf=args.min_leaf, class_weight=cw)
    return GBM(K, rounds=args.rounds, depth=args.depth, lr=args.lr, min_leaf=args.min_leaf, class_weight=cw)


def balanced_weights(y, K):
    cnt = [max(sum(1 for v in y if v == k), 1) for k in range(K)]
    n = len(y)
    return [n / (K * c) for c in cnt]


def decide(probs, K, machine_min):
    """argmax, except that `machine` needs at least machine_min probability (regret trade-off)."""
    out = []
    for p in probs:
        k = max(range(K), key=lambda j: p[j])
        if k == 0 and p[0] < machine_min:
            k = max(range(1, K), key=lambda j: p[j])
        out.append(k)
    return out


def metrics(rows, y, pred, K, names, cost):
    n = len(y)
    acc = sum(1 for a, b in zip(y, pred) if a == b) / max(n, 1)
    conf = [[0] * K for _ in range(K)]
    for a, b in zip(y, pred):
        conf[a][b] += 1
    f1 = []
    for k in range(K):
        tp = conf[k][k]
        fp = sum(conf[j][k] for j in range(K)) - tp
        fn = sum(conf[k]) - tp
        f1.append(2 * tp / max(2 * tp + fp + fn, 1))
    # regret: Claude minutes each row would cost (by its true tier when known)
    def cmin(r):
        t = r["tier"]
        return claude_minutes(r["size"], 4 if t == "4" else 5 if t in ("5", "6") else 0, cost)
    too_exp = [r for r, a, b in zip(rows, y, pred) if a == 0 and b != 0]     # machine sent to Claude
    too_cheap = [r for r, a, b in zip(rows, y, pred) if a != 0 and b == 0]   # Claude sent to a machine tier
    saved = [r for r, a, b in zip(rows, y, pred) if a == 0 and b == 0]
    oracle = [r for r, a in zip(rows, y) if a == 0]
    # among Claude rows, short predicted long and vice versa (K == 3)
    s_as_l = sum(1 for a, b in zip(y, pred) if a == 1 and b == 2)
    l_as_s = sum(1 for a, b in zip(y, pred) if a == 2 and b == 1)
    buckets = {}
    for name, _lo, _hi in SIZE_BUCKETS:
        sel = [(a, b) for r, a, b in zip(rows, y, pred) if size_bucket(r["size"]) == name]
        buckets[name] = (len(sel), sum(1 for a, b in sel if a == b) / max(len(sel), 1))
    return {
        "n": n, "acc": acc, "conf": conf, "f1": f1, "macro_f1": sum(f1) / K,
        "too_exp_n": len(too_exp), "too_exp_kb": sum(r["size"] for r in too_exp) / 1024.0,
        "too_exp_min": sum(cmin(r) for r in too_exp),
        "too_cheap_n": len(too_cheap), "too_cheap_cpu": len(too_cheap) * cost["permuter_cpu"],
        "saved_min": sum(cmin(r) for r in saved), "oracle_min": sum(cmin(r) for r in oracle),
        "short_as_long": s_as_l, "long_as_short": l_as_s, "buckets": buckets,
    }


def print_metrics(title, m, names, cost):
    K = len(names)
    print("  %s: n=%d  accuracy %.3f  macro-F1 %.3f  (%s)"
          % (title, m["n"], m["acc"], m["macro_f1"],
             ", ".join("%s F1 %.2f" % (nm, f) for nm, f in zip(names, m["f1"]))))
    print("    confusion (rows = true, cols = predicted): " +
          "; ".join("%s: %s" % (names[a], " ".join("%4d" % m["conf"][a][b] for b in range(K))) for a in range(K)))
    print("    by size: " + "  ".join("%s n=%d acc %.3f" % (b, v[0], v[1]) for b, v in m["buckets"].items()))
    print("    regret: too expensive (machine-closable sent to Claude) %d functions, %.1f KB, ~%.0f Claude min;"
          " too cheap (Claude work sent to a machine tier) %d functions, ~%.0f CPU min wasted"
          % (m["too_exp_n"], m["too_exp_kb"], m["too_exp_min"], m["too_cheap_n"], m["too_cheap_cpu"]))
    if K == 3:
        print("    Claude length: short routed long %d, long routed short %d" % (m["short_as_long"], m["long_as_short"]))
    print("    Claude minutes saved vs all-to-Claude: %.0f of %.0f possible (%.2f of %.2f windows at %.0f min/window)"
          % (m["saved_min"], m["oracle_min"], m["saved_min"] / cost["window_minutes"],
             m["oracle_min"] / cost["window_minutes"], cost["window_minutes"]))


def run_task(task, rows, args, cost):
    trows, y, K, names = task_rows(rows, task, args.claude_since)
    fnames = feature_names(task)
    X = matrix(trows, fnames)
    kinds = ["rules", "tree", "gbm"] if args.model == "all" else [args.model]
    print("\n== task %s: %d functions, %d bodies; classes %s"
          % (task, len(trows), len(set(r["group"] for r in trows)),
             ", ".join("%s %d" % (nm, sum(1 for v in y if v == k)) for k, nm in enumerate(names))))
    print("   features: %s" % ("static (functions.tsv + image words)" if not task.startswith("draft")
                                else "static + best-draft closeness"))
    # fixed holdout
    fold = group_split(trows, args.holdout, args.seed)
    tr = [i for i, r in enumerate(trows) if fold[r["group"]] != 0]
    te = [i for i, r in enumerate(trows) if fold[r["group"]] == 0]
    results = {}
    for kind in kinds:
        model = make_model(kind, K, fnames, task, args)
        if args.balance and kind != "rules":
            model.class_weight = balanced_weights([y[i] for i in tr], K)
        model.fit([X[i] for i in tr], [y[i] for i in tr])
        pred = decide(model.predict_proba([X[i] for i in te]), K, args.machine_min)
        m = metrics([trows[i] for i in te], [y[i] for i in te], pred, K, names, cost)
        title = "%s, holdout (%d train / %d test bodies)" % (
            kind, len(set(trows[i]["group"] for i in tr)), len(set(trows[i]["group"] for i in te)))
        print_metrics(title, m, names, cost)
        if kind == "rules":
            print("    rules: " + model.describe())
        if kind == "tree" and args.show_tree:
            for line in model.rules(fnames)[:40]:
                print("      " + line)
        results[kind] = m
    # grouped k-fold
    if args.folds and args.folds > 1:
        fold = group_split(trows, 0, args.seed + 1, folds=args.folds)
        for kind in kinds:
            allp, ally, allr = [], [], []
            for k in range(args.folds):
                tr = [i for i, r in enumerate(trows) if fold[r["group"]] != k]
                te = [i for i, r in enumerate(trows) if fold[r["group"]] == k]
                model = make_model(kind, K, fnames, task, args)
                if args.balance and kind != "rules":
                    model.class_weight = balanced_weights([y[i] for i in tr], K)
                model.fit([X[i] for i in tr], [y[i] for i in tr])
                allp += decide(model.predict_proba([X[i] for i in te]), K, args.machine_min)
                ally += [y[i] for i in te]
                allr += [trows[i] for i in te]
            m = metrics(allr, ally, allp, K, names, cost)
            print_metrics("%s, %d-fold grouped CV (every function held out once)" % (kind, args.folds), m, names, cost)
            results[kind + "_cv"] = m
    return results


def open_rows(features_path, ledger_path, min_instructions=rank.MIN_INSTRUCTIONS):
    done = ledger.load(ledger_path)
    out = []
    for r in read_tsv(features_path):
        key = (r["unit"], r["vaddr"])
        if key in done or float(r["instructions"]) < min_instructions:
            continue
        for k in FEATURE_COLS:
            r[k] = float(r.get(k, -1) or -1)
        r["size"] = float(r["size"])
        out.append(r)
    return out


def fit_full(rows, task, args, kind="gbm"):
    trows, y, K, names = task_rows(rows, task, args.claude_since)
    fnames = feature_names(task)
    model = make_model(kind, K, fnames, task, args)
    if args.balance and kind != "rules":
        model.class_weight = balanced_weights(y, K)
    model.fit(matrix(trows, fnames), y)
    return model, fnames, names


def main_eval(args):
    cost = dict(COST)
    cost["window_minutes"] = args.window_minutes
    cost["claude_short_per_kb"], cost["claude_long_per_kb"] = args.claude_min_per_kb
    rows = load_dataset(args.labels, args.features)
    tiers = collections.Counter(r["tier"] for r in rows)
    print("labelled functions with features: %d (%s); twins excluded" % (
        len(rows), ", ".join("tier %s %d" % kv for kv in sorted(tiers.items()))))
    print("learner backend: %s" % backend_description(args))
    if args.claude_since:
        old = sum(1 for r in rows if r["tier"] not in ("1", "2", "3") and r["when"] < args.claude_since)
        print("static tasks leave out %d Claude rows matched before %s (no sweep existed yet, so tier 1 was "
              "never tried on them; --claude-since '' keeps them)" % (old, args.claude_since))
    print("cost model: Claude %.0f/%.0f min per KB (short/long, floors %.0f/%.0f), permuter %.0f CPU min per body, "
          "%.0f Claude min per usage window" % (cost["claude_short_per_kb"], cost["claude_long_per_kb"],
                                                 cost["claude_short_floor"], cost["claude_long_floor"],
                                                 cost["permuter_cpu"], cost["window_minutes"]))
    tasks = args.task or ["static2", "static3", "draft2", "draft3"]
    for task in tasks:
        run_task(task, rows, args, cost)
    if args.no_open:
        return 0
    # projection on the open set
    opens = open_rows(args.features, args.ledger)
    print("\n== open set: %d non-PsyQ functions without a ledger row, %d bodies, %.0f KB"
          % (len(opens), len(set(r["group"] for r in opens)), sum(r["size"] for r in opens) / 1024.0))
    model, fnames, names = fit_full(rows, "static2", args)
    p_static = model.predict_proba(matrix(opens, fnames))
    model_d, fnames_d, _ = fit_full(rows, "draft2", args)
    with_draft = [r for r in opens if r["has_draft"]]
    p_draft = dict(zip((r["unit"] + r["vaddr"] for r in with_draft),
                       model_d.predict_proba(matrix(with_draft, fnames_d)))) if with_draft else {}
    n_mach = sum(1 for p in p_static if p[0] >= args.machine_min)
    exp_min = sum(p[0] * claude_minutes(r["size"], 0, cost) for r, p in zip(opens, p_static))
    all_min = sum(claude_minutes(r["size"], 0, cost) for r in opens)
    print("  static model: %d of %d predicted machine-closable (P >= %.2f); expected Claude minutes avoided"
          " %.0f of %.0f (%.1f of %.1f windows) if every machine tier were still untried"
          % (n_mach, len(opens), args.machine_min, exp_min, all_min,
             exp_min / cost["window_minutes"], all_min / cost["window_minutes"]))
    tried = [r for r in opens if r["has_draft"]]
    print("  but %d of them (or their twin body) already went through a sweep (a draft closeness is on file: %s),"
          " and the overnight permuter ran on the closest of those; for them tier 1 is spent" % (
              len(tried), ", ".join(sorted(set(r["closeness_src"] for r in tried if r["closeness_src"])))))
    if p_draft:
        n2 = sum(1 for r in with_draft if p_draft[r["unit"] + r["vaddr"]][0] >= args.machine_min)
        m2 = sum(p_draft[r["unit"] + r["vaddr"]][0] * claude_minutes(r["size"], 0, cost) for r in with_draft)
        print("  post-sweep model on those %d: %d predicted closable by levers/permuter/golem, expected Claude"
              " minutes avoided %.0f (%.1f windows)" % (len(with_draft), n2, m2, m2 / cost["window_minutes"]))
    untried = [(r, p) for r, p in zip(opens, p_static) if not r["has_draft"]]
    m3 = sum(p[0] * claude_minutes(r["size"], 0, cost) for r, p in untried)
    print("  %d open functions have no draft on file (no sweep report covers them or their body): expected Claude"
          " minutes avoided by running the sweep first %.0f (%.1f windows)"
          % (len(untried), m3, m3 / cost["window_minutes"]))
    return 0


# ---------------------------------------------------------------------------
# rank
# ---------------------------------------------------------------------------
def load_permute_reports(paths):
    out = {}
    for p in paths:
        for r in read_tsv(p):
            if not r.get("unit"):
                continue
            key = (r["unit"], norm_vaddr(r["vaddr"]))
            best = r.get("perm_best") or r.get("base") or r.get("words") or ""
            try:
                best = int(float(best))
            except ValueError:
                best = -1
            prev = out.get(key)
            if prev is None or (0 <= best < prev[0]) or prev[0] < 0:
                out[key] = (best, r.get("status", ""), os.path.basename(os.path.dirname(p)))
    return out


def load_near(path):
    out = {}
    if path and os.path.isfile(path):
        for r in read_tsv(path):
            out[(r["unit"], norm_vaddr(r["vaddr"]))] = (int(r["words"]), int(r.get("size_delta") or 0), r.get("source", ""))
    return out


def recommend(r, p_static, p_draft, perm, near, args):
    """-> (tier, reason).  Cheapest tier that is both likely and not yet spent on this body."""
    tried_sweep = bool(r["has_draft"])
    tried_perm = perm is not None and perm[1] in ("no match", "permuter failed")
    near_words = near[0] if near else -1
    p_m = p_draft[0] if p_draft else p_static[0]
    p_short = (p_draft[1] if p_draft and len(p_draft) > 2 else p_static[1]) if len(p_static) > 2 else None
    if 0 <= near_words <= args.near_max:
        return 2, "agent near miss %d words: permuter on the agent's C, then golem" % near_words
    if perm is not None and 0 <= perm[0] <= args.near_max and tried_perm:
        return 3 if args.golem else 4, "permuter left %d words (%s): %s" % (
            perm[0], perm[2], "golem tweak" if args.golem else "a short Claude pass from the permuter's best text")
    if not tried_sweep:
        if p_static[0] >= args.machine_min:
            return 1, "no draft on file and P(machine) %.2f: sweep first" % p_static[0]
        return (4 if (p_short or 0) >= 0.5 else 5), "no draft on file, P(machine) %.2f: Claude" % p_static[0]
    if tried_sweep and not tried_perm and p_m >= args.machine_min:
        return 2, "sweep missed (closeness %.2f), P(machine) %.2f: permuter" % (r["closeness"], p_m)
    if r["size"] >= args.giant:
        return 5, "giant (%d B): Claude long; candidate for block-region splitting" % int(r["size"])
    return (4 if (p_short or 0) >= 0.5 else 5), "machine tiers spent or unlikely (P(machine) %.2f): Claude" % p_m


def backend_description(args):
    b = backend_name(args)
    if b == "sklearn":
        import sklearn
        return "scikit-learn %s (HistGradientBoostingClassifier / DecisionTreeClassifier)" % sklearn.__version__
    return "pure Python (binned CART / boosted ensemble)%s" % (
        "" if args.backend == "python" else "; numpy + scikit-learn not importable")


def main_rank(args):
    rows = load_dataset(args.labels, args.features)
    opens = open_rows(args.features, args.ledger)
    print("learner backend: %s" % backend_description(args))
    # newer sweep reports override the closeness columns of the open rows
    reports = load_reports(list(args.report or []))
    for r in opens:
        d, src = draft_features((r["unit"], r["vaddr"]), reports)
        if d["has_draft"]:
            r.update(d)
            r["closeness_src"] = src
            r["diff_words"] = int(round((1.0 - d["closeness"]) * r["instructions"]))
    perm = load_permute_reports(list(args.permute_report or []))
    near = load_near(args.near)
    m_static, f_static, _ = fit_full(rows, "static3", args)
    m_draft, f_draft, _ = fit_full(rows, "draft3", args)
    p_static = m_static.predict_proba(matrix(opens, f_static))
    with_draft = [r for r in opens if r["has_draft"]]
    p_draft = dict(zip((r["unit"] + r["vaddr"] for r in with_draft),
                       m_draft.predict_proba(matrix(with_draft, f_draft)))) if with_draft else {}
    out = []
    for r, ps in zip(opens, p_static):
        key = (r["unit"], r["vaddr"])
        pd = p_draft.get(r["unit"] + r["vaddr"])
        pm = perm.get(key)
        nr = near.get(key)
        tier, why = recommend(r, ps, pd, pm, nr, args)
        p = pd or ps
        model_tier = [1, 4, 5][max(range(3), key=lambda k: p[k])]
        out.append({
            "tier": tier, "unit": r["unit"], "vaddr": r["vaddr"], "name": r["name"], "size": int(r["size"]),
            "group": r["group"], "n_members": int(r["n_members"]), "instructions": int(r["instructions"]),
            "score": r["score"], "tier_model": model_tier,
            "p_machine": p[0], "p_short": p[1], "p_long": p[2],
            "closeness": r["closeness"] if r["has_draft"] else "", "closeness_src": r["closeness_src"],
            "tried_sweep": int(bool(r["has_draft"])),
            "permuter_best": pm[0] if pm else "", "permuter_status": pm[1] if pm else "",
            "near_words": nr[0] if nr else "", "near_source": nr[2] if nr else "",
            "why": why,
        })
    out.sort(key=lambda o: (o["tier"], -o["n_members"], o["size"], o["unit"], o["vaddr"]))
    cols = ["tier", "unit", "vaddr", "name", "size", "group", "n_members", "instructions", "score", "tier_model",
            "p_machine", "p_short", "p_long", "closeness", "closeness_src", "tried_sweep", "permuter_best",
            "permuter_status", "near_words", "near_source", "why"]
    write_tsv(args.out, out, cols,
              "router.py rank: open functions with the tier to try next (1 sweep, 2 permuter, 3 golem, 4 Claude short, "
              "5 Claude long); tier_model / p_* from the boosted model, tier from the model plus what is already spent")
    cnt = collections.Counter(o["tier"] for o in out)
    kb = collections.defaultdict(float)
    for o in out:
        kb[o["tier"]] += o["size"] / 1024.0
    print("wrote %s: %d open functions" % (os.path.relpath(args.out, REPO), len(out)))
    for t in sorted(cnt):
        print("  tier %d %-13s %4d functions %7.1f KB" % (t, TIER_NAME[t], cnt[t], kb[t]))
    return 0


# ---------------------------------------------------------------------------
def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    def common(p):
        p.add_argument("--functions", default=rank.FUNCTIONS)
        p.add_argument("--ledger", default=LEDGER)
        p.add_argument("--labels", default=LABELS)
        p.add_argument("--features", default=FEATURES)

    def model_opts(p):
        p.add_argument("--rounds", type=int, default=60, help="boosting rounds (default 60)")
        p.add_argument("--depth", type=int, default=3, help="boosted tree depth (default 3)")
        p.add_argument("--lr", type=float, default=0.1)
        p.add_argument("--tree-depth", type=int, default=5, help="single CART depth (default 5)")
        p.add_argument("--min-leaf", type=int, default=10)
        p.add_argument("--balance", action="store_true", help="class-balanced weights")
        p.add_argument("--machine-min", type=float, default=0.5,
                       help="route to a machine tier only above this probability (default 0.5)")
        p.add_argument("--claude-since", default="2026-10-20",
                       help="static tasks: leave out Claude rows matched before this date, when no sweep had "
                            "been run on them (tier 1 untried); '' keeps them")
        p.add_argument("--backend", choices=["auto", "python", "sklearn"], default="auto",
                       help="learners: scikit-learn (HistGradientBoosting / DecisionTree) when it imports, "
                            "else the pure-Python ones (default auto)")

    p = sub.add_parser("labels", help="derive the closing tier per ledger row")
    common(p)
    p.add_argument("--short-max", type=int, default=2, help="Claude short = at most this many tries (default 2)")
    p.add_argument("--out", default=LABELS)

    p = sub.add_parser("features", help="static + image + draft features per function (needs the images)")
    common(p)
    p.add_argument("--report", action="append", help="extra sweep_report.tsv (newest last)")
    p.add_argument("--diff", help="diff table from compiled drafts: unit vaddr words reg_only size_delta")
    p.add_argument("--out", default=FEATURES)

    p = sub.add_parser("eval", help="held-out tier accuracy, regret, projected savings")
    common(p)
    model_opts(p)
    p.add_argument("--model", choices=["rules", "tree", "gbm", "all"], default="all")
    p.add_argument("--task", action="append", choices=["static2", "static3", "draft2", "draft3"])
    p.add_argument("--holdout", type=float, default=0.2)
    p.add_argument("--folds", type=int, default=5)
    p.add_argument("--seed", type=int, default=1)
    p.add_argument("--show-tree", action="store_true", help="print the CART rules")
    p.add_argument("--no-open", action="store_true", help="skip the open-set projection")
    p.add_argument("--window-minutes", type=float, default=COST["window_minutes"])
    p.add_argument("--claude-min-per-kb", type=float, nargs=2, metavar=("SHORT", "LONG"),
                   default=(COST["claude_short_per_kb"], COST["claude_long_per_kb"]))

    p = sub.add_parser("rank", help="queue TSV of the open functions with the tier to try next")
    common(p)
    model_opts(p)
    p.add_argument("--report", action="append", help="newer sweep_report.tsv for the open bodies")
    p.add_argument("--permute-report", action="append", help="permute_report.tsv of the permuter passes")
    p.add_argument("--near", default=NEAR, help="agent near misses (default tools/loop/near_misses.tsv)")
    p.add_argument("--near-max", type=int, default=12, help="a near miss within this many words goes to the permuter")
    p.add_argument("--giant", type=int, default=4096, help="bodies this large go to Claude long with a split note")
    p.add_argument("--golem", action="store_true", help="route permuter leftovers to the golem (GOLEM=1) instead of Claude")
    p.add_argument("--out", default=os.path.join(HERE, "queue-router.tsv"))
    args = ap.parse_args(argv)
    return {"labels": main_labels, "features": main_features, "eval": main_eval, "rank": main_rank}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
