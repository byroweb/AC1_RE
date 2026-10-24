#!/usr/bin/env python3
"""doctor.py - one front door: compile a near miss once, say what kind of miss it is,
and run (or name) the tool that fits.

    doctor.py check UNIT VADDR [CFILE]           classify one function and print the plan
    doctor.py fix   UNIT VADDR [CFILE] [--budget S] [--no-ledger] [--dry-run]
                                                 classify, then run the plan's tools in order
    doctor.py scan  (--cfile UNIT:C ... | --list FILE)  [--out TSV]
                                                 classify many (one compile each): a census

CFILE defaults to re/match/<unit>/func_<ADDR>.c.  Everybody can call this instead of
choosing among sweep / levers / regtie / permuter / twins by hand: agents ("write C, run
doctor"), golems, overnight.sh, a person in the workbench.

Classes (first that applies), from one oracle-identical compile and the retail words:

  build      cc1 / cpp / as fails: the error, then m2c fix-ups or an agent
  match      the bytes already equal retail: `drive.py try` commits it
  twin       a member of the body's twin group is matched: `twins.py replay`
  size-small at most 16 bytes off with the same branch skeleton: a delay slot, a spill
  size       our function is longer or shorter: structural (blocks added or lost)
  structure  same size, but the branch/call skeleton differs: control flow, layout,
             cross-jumping, switch lowering
  constants  every differing word differs only in an immediate: levers imm/cmp
  registers  only register names differ, consistently (a tie in global-alloc): regtie
  schedule   the same instructions in another order (after the register renaming):
             the scheduler / delay slots: permuter
  mixed      registers and order and a little else: regtie, then the permuter

Plans escalate cheapest first: levers (seconds) -> regtie (a minute) -> one permuter
job (--budget s) -> an agent.  Every `fix` appends one row per tool tried to the doctor
log ($AC1_DOCTOR_LOG, default ~/dev/ac1_work/doctor_log.tsv: when, unit, vaddr, class,
words, tool, result, note): the outcome data the router (router.py) and a fine-tuned
model need.  A tool's match is judged by the oracle (drive.py try) before it counts.
"""
import argparse
import collections
import datetime
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
import match_fn  # noqa: E402
import permute  # noqa: E402
import regtie  # noqa: E402

LOG = os.environ.get("AC1_DOCTOR_LOG") or os.path.join(os.path.expanduser("~"), "dev", "ac1_work", "doctor_log.tsv")
LOG_COLS = ["when", "unit", "vaddr", "class", "words", "tool", "result", "note"]
BRANCH_OPS = re.compile(r"^(b\w*|j|jal|jalr|jr|beqz|bnez|bltz|bgez|blez|bgtz|bal)$")
NUM = re.compile(r"^-?(0x[0-9a-f]+|\d+)$")

PLANS = {
    "build": ["agent"],
    "match": ["commit"],
    "twin": ["twins", "regtie", "permuter"],
    "size": ["structdiff", "regtie", "agent"],
    "size-small": ["regtie", "permuter", "structdiff"],
    "structure": ["structdiff", "regtie", "agent"],
    "constants": ["levers", "regtie", "permuter"],
    "registers": ["regtie", "permuter"],
    "schedule": ["permuter", "regtie"],
    "mixed": ["regtie", "levers", "permuter"],
}
WHY = {
    "build": "the C does not compile; fix the error (m2cfix.py does the common m2c ones) or hand it to an agent",
    "match": "already byte-identical: commit it with drive.py try",
    "twin": "a near-copy of this body is matched: replay that C here (twins.py replay)",
    "size": "the function has a different length: blocks are added or lost (control flow, cross-jumping, "
            "a duplicated tail, an if-chain vs switch)",
    "size-small": "a few words longer or shorter with the same branches: a delay slot left empty or filled, "
                  "an extra spill or a hoisted constant (usually a register-allocation effect)",
    "structure": "same length, but branches/calls sit elsewhere: control flow or block layout differs",
    "constants": "only immediates differ: the imm/cmp levers put retail's constants and comparisons in",
    "registers": "only register names differ, as a consistent swap: a global-alloc tie (regtie reads and steers it)",
    "schedule": "the same instructions in another order: scheduling / delay-slot filling (the permuter's ground)",
    "mixed": "register names and order and a few other words: a tie plus its knock-on effects",
}


# ---------------------------------------------------------------------------
# classify
# ---------------------------------------------------------------------------
def build_error(meta, text, inc):
    """The first lines of the toolchain's complaint, for a text that does not build."""
    work = tempfile.mkdtemp(prefix="doctor_err_")
    try:
        c = os.path.join(work, "fn.c")
        with open(c, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        try:
            permute.preprocess(meta["tools"], c, os.path.join(work, "fn.i"), [inc])
            permute.run(match_fn.cc1_command(meta["tools"]["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"],
                        work)
        except RuntimeError as e:
            return " ".join(str(e).split())[:300]
        return "assembles or links badly (cc1 is fine)"
    finally:
        shutil.rmtree(work, ignore_errors=True)


def norm(row):
    op, toks = regtie.split_ops(row)
    return op, toks


def word_kind(o, r):
    """How one aligned pair of instructions differs: reg | imm | op | mixed."""
    oo, ot = norm(o)
    ro, rt = norm(r)
    if oo != ro or len(ot) != len(rt):
        return "op"
    kinds = set()
    for x, y in zip(ot, rt):
        if x == y:
            continue
        if regtie.REG_TOKEN.match(x) and regtie.REG_TOKEN.match(y):
            kinds.add("reg")
        elif NUM.match(x) and NUM.match(y):
            kinds.add("imm")
        else:
            kinds.add("other")
    if not kinds:
        return "same"
    return kinds.pop() if len(kinds) == 1 else "mixed"


def skeleton(rows):
    """The branch/jump/call opcodes in order: a coarse control-flow signature."""
    return [norm(r)[0] for r in rows if BRANCH_OPS.match(norm(r)[0])]


def rename(rows, mapping):
    out = []
    for r in rows:
        op, toks = norm(r)
        out.append((op, tuple(mapping.get(t, t) for t in toks)))
    return out


def classify(meta, retail, res):
    """-> (class, words, facts dict)."""
    if res is None:
        return "build", None, {}
    ours = res["bytes"]
    words = permute.word_score(ours, retail)
    if words == 0:
        return "match", 0, {}
    our_rows = regtie.disasm_rows(meta, ours)
    ret_rows = regtie.disasm_rows(meta, retail)
    facts = {"size": (len(ours), len(retail))}
    if len(ours) != len(retail):
        facts["bytes_off"] = len(ours) - len(retail)
        same_skel = skeleton(our_rows) == skeleton(ret_rows)
        facts["same_skeleton"] = same_skel
        if same_skel and abs(len(ours) - len(retail)) <= 16:
            # a word or few more/less, same branches: a delay slot, a spill, a hoisted constant
            return "size-small", words, facts
        return "size", words, facts
    if skeleton(our_rows) != skeleton(ret_rows):
        return "structure", words, facts
    kinds = collections.Counter(word_kind(o, r) for o, r in zip(our_rows, ret_rows))
    kinds.pop("same", None)
    facts["kinds"] = dict(kinds)
    if set(kinds) == {"imm"}:
        return "constants", words, facts
    # a consistent register renaming: ours -> retail, one target per register
    pairs = regtie.reg_map(ret_rows, our_rows)
    mapping, clash = {}, False
    for (o, r), _n in pairs.most_common():
        if o in mapping and mapping[o] != r:
            clash = True
        mapping.setdefault(o, r)
    facts["renames"] = ", ".join("%s->%s" % kv for kv in sorted(mapping.items()))
    renamed = rename(our_rows, mapping)
    target = [(op, tuple(t)) for op, t in (norm(r) for r in ret_rows)]
    if set(kinds) == {"reg"}:
        # every differing word differs only in register names: a tie (consistent or local)
        return "registers", words, facts
    if renamed == target and not clash:
        return "registers", words, facts
    if collections.Counter(renamed) == collections.Counter(target):
        return "schedule" if not mapping else "mixed", words, facts
    ops = kinds.get("op", 0) + kinds.get("mixed", 0) + kinds.get("other", 0)
    facts["non_register_words"] = ops
    if set(kinds) <= {"imm", "reg"}:
        return "mixed", words, facts
    if ops <= max(4, words // 4):
        # a few real instruction changes in a same-skeleton body: scheduling, delay slots,
        # a hoisted constant, usually riding on a register tie
        return ("mixed" if kinds.get("reg") else "schedule"), words, facts
    return "structure", words, facts


def twin_matched(unit, vaddr):
    """'unit vaddr' of a matched member of this body's twin group, or None."""
    import progress
    import twins
    matched = progress.load_matched(os.path.join(REPO, "re", "match", "manifest.tsv"))
    for path in (twins.TWINS_PROGRAMS, twins.TWINS):
        if not os.path.isfile(path):
            continue
        groups = collections.defaultdict(list)
        mine = None
        for line in open(path, encoding="utf-8"):
            if line.startswith("#"):
                continue
            p = line.rstrip("\n").split("\t")
            if len(p) < 5:
                continue
            key = (p[3], int(p[4], 16))
            groups[p[0]].append(key)
            if key == (unit, vaddr):
                mine = p[0]
        if mine:
            for k in groups[mine]:
                if k != (unit, vaddr) and k in matched:
                    return "%s 0x%08X" % k
    return None


# ---------------------------------------------------------------------------
# tools
# ---------------------------------------------------------------------------
def append_log(row):
    os.makedirs(os.path.dirname(LOG) or ".", exist_ok=True)
    new = not os.path.isfile(LOG)
    with open(LOG, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(LOG_COLS) + "\n")
        fh.write("\t".join(str(row.get(c, "")).replace("\t", " ").replace("\n", " ") for c in LOG_COLS) + "\n")


def oracle_commit(unit, vaddr, text, how, no_ledger):
    """Write text to re/match and judge it with drive.py try (manifest + ledger on a clean MATCH)."""
    return regtie.commit(unit, vaddr, text, [how], no_ledger)


def run_levers(st, args):
    import levers
    lv = permute.lever_search(st["meta"], st["syms"], st["text"], st["retail"], st["inc"], levers.DEFAULT_LEVERS,
                              300, 4)
    if lv["best"] == 0:
        return "zero", lv["text"], "levers " + " + ".join(lv["trail"])
    return "best %s" % lv["best"], (lv["text"] if lv["best"] is not None and lv["best"] < st["words"] else None), ""


def run_regtie(st, args):
    out = regtie.search(st["meta"], st["syms"], st["text"], st["inc"], st["retail"], evals=args.evals, beam=8,
                        log=lambda m: None)
    if out.get("words") == 0:
        return "zero", out["text"], "regtie " + " + ".join(out["trail"])
    better = out.get("text") if out.get("words") is not None and out["words"] < st["words"] else None
    return "best %s" % out.get("words"), better, " + ".join(out.get("trail", []))[:200]


def run_permuter(st, args):
    spec = "%s:%s" % (st["unit"], st["cfile"])
    cmd = [sys.executable, os.path.join(HERE, "permute.py"), "run", "-j", "1", "--budget", str(args.budget),
           "--no-replay", "--cfile", spec, "--work", os.path.join(st["work"], "perm")]
    if args.no_ledger:
        cmd.append("--no-ledger")
    if args.dry_run:
        return "skipped (dry run)", None, ""
    r = subprocess.run(cmd, cwd=REPO, capture_output=True, text=True, errors="replace")
    line = [l for l in r.stdout.splitlines() if l.startswith("[")]
    status = "MATCH" if line and " MATCH " in line[-1] else "no match"
    return status, None, (line[-1][-120:] if line else r.stdout[-200:])


def run_twins(st, args):
    if args.dry_run:
        return "skipped (dry run)", None, ""
    r = subprocess.run([sys.executable, os.path.join(HERE, "twins.py"), "replay", "--scope", "all", "-j", "4"],
                       cwd=REPO, capture_output=True, text=True, errors="replace")
    ok = any(l.startswith("manifest:") and not l.startswith("manifest: 0 ") for l in r.stdout.splitlines())
    return ("replayed" if ok else "no new rows"), None, ""


def run_structdiff(st, args):
    """The structural findings, then structdiff's moves at the flagged lines (an improved
    text goes on to the next tool)."""
    import structdiff
    lm, head, fs = structdiff.run(st["unit"], st["vaddr"], st["cfile"])
    print("    " + structdiff.render(lm, head, fs).replace("\n", "\n    "))
    kinds = collections.Counter(k for k, _m, _x in fs if k not in ("divergence", "regions"))
    note = ", ".join("%s %d" % kv for kv in kinds.most_common()) or "no named finding"
    out = structdiff.fix(st["unit"], st["vaddr"], st["cfile"], evals=min(args.evals, 300), log=lambda m: None)
    if out.get("words") == 0:
        return "zero", out["text"], "structdiff " + " + ".join(out["trail"])
    better = out.get("text") if out.get("words") is not None and out["words"] < st["words"] else None
    if better:
        note += "; moves %s -> %s: %s" % (st["words"], out["words"], " + ".join(out["trail"])[:120])
    return "findings", better, note


TOOLS = {"levers": run_levers, "regtie": run_regtie, "permuter": run_permuter, "twins": run_twins,
         "structdiff": run_structdiff}


# ---------------------------------------------------------------------------
def default_cfile(unit, vaddr):
    return os.path.join(REPO, "re", "match", unit, "func_%08X.c" % vaddr)


def examine(unit, vaddr, cfile, work):
    meta, syms, retail, text = regtie.load(unit, vaddr, cfile, work)
    inc = os.path.dirname(os.path.abspath(cfile))
    res = regtie.build(meta, syms, text, inc)
    cls, words, facts = classify(meta, retail, res)
    if res is not None and res.get("unresolved"):
        facts["unresolved"] = ", ".join(sorted(res["unresolved"]))
    if cls not in ("match", "build"):
        tw = twin_matched(unit, vaddr)
        if tw:
            facts["twin"] = tw
            cls = "twin"
    if cls == "build":
        facts["error"] = build_error(meta, text, inc)
    return {"unit": unit, "vaddr": vaddr, "cfile": cfile, "meta": meta, "syms": syms, "retail": retail,
            "text": text, "inc": inc, "res": res, "class": cls, "words": words, "facts": facts, "work": work}


def report(st):
    lines = ["%s 0x%08X: %s%s" % (st["unit"], st["vaddr"], st["class"],
                                  "" if st["words"] is None else " (%d differing words)" % st["words"])]
    lines.append("  why:  " + WHY[st["class"]])
    for k, v in sorted(st["facts"].items()):
        lines.append("  %s: %s" % (k, v))
    lines.append("  plan: " + " -> ".join(PLANS[st["class"]]))
    if st["class"] in ("registers", "mixed") and st["res"] is not None:
        lines.append("  " + regtie.describe(regtie.Picture(st["meta"], st["res"], st["retail"])).replace("\n", "\n  "))
    return "\n".join(lines)


def resolve(args):
    """unit, vaddr, cfile from the arguments; CFILE may carry @rodata=VA / @noalign."""
    unit, vaddr = args.unit, int(args.vaddr, 16)
    cfile = args.cfile or default_cfile(unit, vaddr)
    if "@" in cfile:
        _u, _v, cfile = regtie.parse_cfile("%s:%s" % (unit, cfile))
    return unit, vaddr, cfile


def cmd_check(args):
    unit, vaddr, cfile = resolve(args)
    work = tempfile.mkdtemp(prefix="doctor_")
    try:
        print(report(examine(unit, vaddr, cfile, work)))
    finally:
        shutil.rmtree(work, ignore_errors=True)
    return 0


def cmd_fix(args):
    unit, vaddr, cfile = resolve(args)
    work = tempfile.mkdtemp(prefix="doctor_")
    try:
        st = examine(unit, vaddr, cfile, work)
        print(report(st))
        when = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        base = {"when": when, "unit": unit, "vaddr": "0x%08X" % vaddr, "class": st["class"], "words": st["words"]}
        if st["class"] == "match":
            res = "dry run" if args.dry_run else oracle_commit(unit, vaddr, st["text"], "as written", args.no_ledger)
            print("  commit: " + res)
            append_log(dict(base, tool="commit", result=res))
            return 0 if res in ("MATCH", "dry run") else 1
        skip = {t.strip() for t in args.skip.split(",") if t.strip()}
        for tool in [t for t in PLANS[st["class"]] if t not in skip]:
            if tool not in TOOLS:
                print("  %s: not automatic (%s)" % (tool, "agent work" if tool == "agent" else "not built yet"))
                append_log(dict(base, tool=tool, result="handed off"))
                continue
            print("  trying %s ..." % tool)
            sys.stdout.flush()
            try:
                status, better, note = TOOLS[tool](st, args)
            except Exception as e:  # noqa: BLE001 - next tool
                status, better, note = "crash", None, "%s: %s" % (type(e).__name__, e)
            if status == "zero":
                res = "dry run" if args.dry_run else oracle_commit(unit, vaddr, better, note, args.no_ledger)
                status = "MATCH" if res == "MATCH" else "zero but " + res
            print("    %s %s" % (status, note[:160]))
            append_log(dict(base, tool=tool, result=status, note=note))
            if status in ("MATCH", "replayed", "dry run"):
                return 0
            if better:                                     # the next tool starts from the improvement
                st["text"] = better
                path = os.path.join(work, "func_%08X.c" % vaddr)
                with open(path, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write(better)
                st["cfile"] = path
                m = re.search(r"best (\d+)", status) or re.search(r"-> (\d+):", note)
                if m:
                    st["words"] = int(m.group(1))
                if args.save_best:
                    d = os.path.join(args.save_best, unit)
                    os.makedirs(d, exist_ok=True)
                    shutil.copyfile(path, os.path.join(d, "func_%08X.c" % vaddr))
        return 1
    finally:
        shutil.rmtree(work, ignore_errors=True)


def cmd_scan(args):
    specs = list(args.cfile or [])
    if args.list:
        specs += [l.strip() for l in open(args.list, encoding="utf-8") if l.strip()]
    work = tempfile.mkdtemp(prefix="doctor_scan_")
    rows = []
    try:
        for k, spec in enumerate(specs, 1):
            unit, vaddr, path = regtie.parse_cfile(spec)
            try:
                st = examine(unit, vaddr, path, os.path.join(work, str(k)))
                cls, words, facts = st["class"], st["words"], st["facts"]
            except Exception as e:  # noqa: BLE001
                cls, words, facts = "error", None, {"error": "%s: %s" % (type(e).__name__, e)}
            size = permute.load_meta(os.path.join(work, str(k), "%s_%08X" % (unit, vaddr)))["size"] \
                if os.path.isfile(os.path.join(work, str(k), "%s_%08X" % (unit, vaddr), "meta.json")) else ""
            rows.append([unit, "0x%08X" % vaddr, str(size), cls, "" if words is None else str(words),
                         "; ".join("%s=%s" % kv for kv in sorted(facts.items()))[:300], path])
            print("%-7s 0x%08X %5s B  %-10s %5s  %s" % (unit, vaddr, size, cls, rows[-1][4], rows[-1][5][:90]))
            sys.stdout.flush()
    finally:
        shutil.rmtree(work, ignore_errors=True)
    c = collections.Counter(r[3] for r in rows)
    print("census: " + ", ".join("%s %d" % kv for kv in c.most_common()))
    if args.out:
        with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
            fh.write("unit\tvaddr\tsize\tclass\twords\tfacts\tcfile\n")
            for r in rows:
                fh.write("\t".join(r) + "\n")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("check", "fix"):
        p = sub.add_parser(name)
        p.add_argument("unit")
        p.add_argument("vaddr")
        p.add_argument("cfile", nargs="?")
        if name == "fix":
            p.add_argument("--budget", type=float, default=1200, help="permuter seconds (default %(default)s)")
            p.add_argument("--evals", type=int, default=1500, help="regtie compiles (default %(default)s)")
            p.add_argument("--no-ledger", action="store_true")
            p.add_argument("--dry-run", action="store_true", help="never write re/match")
            p.add_argument("--save-best", metavar="DIR", help="write every improved text to DIR/<unit>/func_<ADDR>.c")
            p.add_argument("--skip", default="", help="tools to leave out of the plan, comma-separated "
                           "(e.g. permuter after an overnight pass)")
    p = sub.add_parser("scan")
    p.add_argument("--cfile", action="append", metavar="UNIT:C")
    p.add_argument("--list", help="file of UNIT:C lines")
    p.add_argument("--out", help="write the census TSV here")
    args = ap.parse_args(argv)
    return {"check": cmd_check, "fix": cmd_fix, "scan": cmd_scan}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
