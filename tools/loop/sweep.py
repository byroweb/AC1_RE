#!/usr/bin/env python3
"""sweep.py - zero-agent sweep: m2c drafts straight into the oracle, no LLM in the loop.

    sweep.py --queue FILE [--queue FILE ...] [--max-size 160] [--limit 100] [-j 20]
             [--variants ctx,ctx-regvars,ctx-knr,...] [--try-all]
             [--work DIR] [--report FILE] [--best-dir DIR] [--dry-run] [--no-replay]
             [--no-ledger]

For each open body in the queue files (make_queues.py --bodies rows, or any
queue with unit/vaddr/size columns), in file order, skipping rows that are no
longer open in the ledger, that already have re/match/<unit>/func_<ADDR>.c, or
that are --max-size bytes or larger:

  1. drafts, in --variants order until one matches: tools/match/ctx.py
     decompile() (symbolised asm + the shared-type context) with m2c
     --valid-syntax (`ctx`), then with --reg-vars saved (`ctx-regvars`), then
     `ctx` with m2c's guessed extern prototypes made unprototyped (`ctx-knr`);
     also known: ctx-no-andor, ctx-void, raw (no context), and the tools/loop/m2cfix.py
     fix-ups: ctx-spdecl (declare the stack/m2c names m2c uses undeclared), ctx-spframe
     (all spNN as fields of one frame union at their offsets), ctx-voidcast (value of a
     void callee, `*(void *)`), ctx-fix / ctx-fix-frame / ctx-fix-regvars (knr + spdecl
     or spframe + voidcast, then up to 6 rounds of m2cfix.repair() on the lines cc1
     rejects); a draft
     gets `#include "ac1/ac1.h"` and `#include "m2c_compat.h"` (the M2C_* names);
  2. each draft goes through tools/match/match_fn.py with the unit's flags,
     --expect-size, --rodata from jt_addrs, the two --syms tables, and every
     unresolved func_<ADDR> / D_<ADDR> as --defsym (rerun once), as
     `drive.py try` does;
  3. a clean MATCH (exit 0, final line MATCH, no masked word, size equal) is
     written to re/match/<unit>/func_<ADDR>.c with a header comment, the oracle
     is run once more on that file, and only on exit 0 the manifest row
     (drive.manifest_append) and a ledger row (match, attempts 0 = unknown,
     reason `zero-agent sweep (<variant>)`) are added.

Jobs run in a thread pool; each job's work is m2c and cc1/maspsx/as
subprocesses, so -j scales with cores.  The manifest and ledger are written by
the main thread only.  At the end `twins.py replay --scope programs` credits
the new matches' twins (--no-replay skips it).  A per-body report (variants,
results, first diff, and the closeness of the best compiling draft: `closeness` =
the oracle's aligned_words / retail words, `positional` = words equal at the same
offset, `best_variant`) goes to --report (default <work>/sweep_report.tsv), a
per-variant table (drafted / compiled / matched / mean closeness) to stdout, and with
--best-dir each missed body's best draft to <dir>/<unit>_<ADDR>.c.
Nothing is recorded for a body that did not match: a miss here says nothing
about how hard the function is for a matcher.
"""
import argparse
import concurrent.futures as cf
import datetime
import os
import re
import shutil
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
import drive   # noqa: E402
import ledger  # noqa: E402
import units   # noqa: E402
import ctx     # noqa: E402
import m2cfix  # noqa: E402
from m2cfix import knr_externs  # noqa: E402,F401  (kept importable from here)

V = "--valid-syntax"
SS = "--stack-structs"


def _knr(text, name, row):
    return m2cfix.knr_externs(text)


def _frame(text, name, row):
    return m2cfix.stack_frame(text, name, row, blob=body_bytes(row))


def _fix(text, name, row):
    return m2cfix.void_casts(m2cfix.declare_locals(m2cfix.knr_externs(text), name, row))


def _fix_frame(text, name, row):
    return m2cfix.void_casts(_frame(m2cfix.knr_externs(text), name, row))


# name -> (raw, m2c args, text fix-ups (text, fn name, row) -> text, repair rounds);
# raw = ctx.decompile(raw=True): no context, no symbol rewrite.  Repair rounds: after a
# cc1 error, m2cfix.repair() patches the lines cc1 names and the oracle runs again, up
# to that many times.  Measured on the misses of the full sweep (README).
VARIANTS = {"ctx": (False, [V], (), 0),
            "ctx-no-andor": (False, [V, "--no-andor"], (), 0),
            "ctx-knr": (False, [V], (_knr,), 0),
            "ctx-regvars": (False, [V, "--reg-vars", "saved"], (), 0),
            "ctx-void": (False, [V, "--void"], (), 0),
            "raw": (True, [V], (), 0),
            "ctx-spdecl": (False, [V, SS], (m2cfix.declare_locals,), 0),
            "ctx-spframe": (False, [V, SS], (_frame,), 0),
            "ctx-voidcast": (False, [V], (m2cfix.void_casts,), 0),
            "ctx-fix": (False, [V, SS], (_fix,), 6),
            "ctx-fix-frame": (False, [V, SS], (_fix_frame,), 6),
            "ctx-fix-regvars": (False, [V, SS, "--reg-vars", "saved"], (_fix,), 6)}
DEFAULT_VARIANTS = "ctx,ctx-regvars,ctx-knr,ctx-spframe,ctx-fix-frame,ctx-fix"
PRELUDE = '#include "ac1/ac1.h"\n#include "m2c_compat.h"\n'


PARAM = re.compile(r"^\s*(.*?)\barg(\d+)\s*$")


def fill_params(text, name):
    """m2c leaves unused leading parameters out of the definition (`f(s32 arg2)` for a
    function that reads only a2), which moves the used ones into the wrong registers.
    Put `s32 argK` back for every missing K below the highest one."""
    m = re.search(r"^(\S[^\n;]*?\b%s)\(([^()]*)\)(\s*\{)" % re.escape(name), text, re.M)
    if not m or m.group(2).strip() in ("", "void"):
        return text
    params = {}
    for p in m.group(2).split(","):
        pm = PARAM.match(p)
        if not pm:
            return text                      # not plain `TYPE argN` params: leave alone
        params[int(pm.group(2))] = p.strip()
    top = max(params)
    if len(params) == top + 1:
        return text
    sig = ", ".join(params.get(k, "s32 arg%d" % k) for k in range(top + 1))
    return text[:m.start(2)] + sig + text[m.end(2):]


def body_bytes(row):
    image, base = ctx.unit_image(row["unit"])
    with open(image, "rb") as fh:
        fh.seek(int(row["vaddr"], 16) - base)
        return fh.read(int(row["size"]))


def draft(row, variant, cache=None):
    """C text of one draft, or raises ctx.CtxError.  cache: dict shared by one body's
    variants, so m2c runs once per distinct (raw, args)."""
    raw, args, fixups, _rounds = VARIANTS[variant]
    key = (raw, tuple(args))
    if cache is not None and key in cache:
        out = cache[key]
    else:
        try:
            out = ctx.decompile(int(row["vaddr"], 16), int(row["size"]), row["unit"], raw=raw,
                                m2c_args=args)
        except ctx.CtxError as e:
            out = e
        if cache is not None:
            cache[key] = out
    if isinstance(out, ctx.CtxError):
        raise out
    if not out.strip():
        raise ctx.CtxError("m2c printed nothing")
    name = drive.fn_name(row["vaddr"])
    for f in fixups:
        out = f(out, name, row)
    if SS in args:                       # the template's types are used; drop the struct
        out = m2cfix.stack_template(out)[0]
    return PRELUDE + "\n" + fill_params(out, name)


CC1_PREFIX = re.compile(r"^\S*cc1\S* failed \(exit \d+\):\s*(?:fn\.i: In function `\w+':\s*)?(?:fn\.i:\d+:\s*)?")


def closeness(facts, size):
    """(positional, aligned) share of the retail words our object reproduces: positional
    = words equal at the same offset (diff_words), aligned = the oracle's aligned_words
    (difflib's in-order pairing, survives an inserted instruction); Nones without an object."""
    if not facts or facts.get("result") not in ("MATCH", "MISMATCH") or not facts.get("bytes"):
        return None, None
    words = size // 4
    same = max(0, min(facts["bytes"], size) // 4 - facts.get("diff_words", 0))
    al = facts.get("aligned_words")
    return min(1.0, same / words), (min(1.0, al / words) if al is not None else None)


def oracle(u, vaddr, cpath, size, rodata, keep=None):
    """(clean?, defsyms, facts-summary, facts) with drive.py try's auto --defsym rerun.
    keep: the oracle's work dir (fn.i for m2cfix.repair()); the caller removes it."""
    defsyms = []
    argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep=keep)
    rc, facts, _text, err = drive.run_oracle(argv)
    if facts and facts.get("unresolved_symbols"):
        auto = []
        for s in facts["unresolved_symbols"]:
            m = drive.AUTO_SYM.match(s)
            if m:
                auto.append("%s=0x%s" % (s, m.group(2).upper()))
        if auto:
            defsyms = auto
            argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep=keep)
            rc, facts, _text, err = drive.run_oracle(argv)
    result = facts["result"] if facts else "ERROR"
    masked = facts.get("masked_words", 0) if facts else 0
    clean = (rc == 0 and result == "MATCH" and masked == 0 and facts.get("bytes") == size)
    first = facts.get("first_diff_offset") if facts else None
    summ = result + ("(masked)" if result == "MATCH" and masked else "")
    if result == "MISMATCH" and first is not None:
        summ += "@0x%x/%s" % (first, facts.get("diff_words"))
    if result == "ERROR":
        msg = (facts or {}).get("error") or err.strip().splitlines()[-1:] or [""]
        msg = " ".join((msg if isinstance(msg, str) else msg[0]).split())
        summ += ":" + CC1_PREFIX.sub("", msg)[:80]
    elif result == "MISMATCH":
        _pos, al = closeness(facts, size)
        if al is not None:
            summ += "~%.2f" % al
    return clean, defsyms, summ, facts


def check(u, vaddr, cpath, text, size, rodata, rounds, name):
    """Oracle on one draft, then up to `rounds` m2cfix.repair() passes while cc1 rejects it.
    -> (clean, defsyms, summary, facts, final text, repairs applied).  The oracle's work
    dir is <draft>.work, removed afterwards (match_fn keeps it on a miss otherwise)."""
    keep = cpath[:-2] + ".work"
    n = 0
    try:
        while True:
            with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(text)
            shutil.rmtree(keep, ignore_errors=True)
            clean, defsyms, summ, facts = oracle(u, vaddr, cpath, size, rodata, keep)
            fn_i = os.path.join(keep, "fn.i")
            if n >= rounds or not summ.startswith("ERROR") or not os.path.exists(fn_i):
                return clean, defsyms, summ, facts, text, n
            errs = m2cfix.map_errors(text, fn_i, m2cfix.cc1_errors((facts or {}).get("error")))
            fixed = m2cfix.repair(text, name, errs)
            if not fixed or fixed == text:
                return clean, defsyms, summ, facts, text, n
            text, n = fixed, n + 1
    finally:
        shutil.rmtree(keep, ignore_errors=True)


def work(row, workdir, variants, try_all=False):
    """Try the variants in order on one body -> dict(result..., c_text, defsyms, variant).
    try_all: keep trying after the first clean one (to measure variants; the first wins).
    res["best"] is the compiling draft with the highest aligned closeness, res["stats"]
    one (variant, match|compiled|error|m2c_fail, positional, aligned) per try."""
    unit, vaddr = row["unit"], int(row["vaddr"], 16)
    size = int(row["size"])
    u = units.resolve(unit)
    rodata = None
    if int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    res = {"row": row, "tries": [], "match": None, "best": None, "stats": []}
    m2c_cache = {}
    for variant in variants:
        try:
            text = draft(row, variant, m2c_cache)
        except ctx.CtxError as e:
            lines = str(e).strip().splitlines() or [""]
            res["tries"].append((variant, "M2C_FAIL:" + " ".join(lines[-1].split())[:80]))
            res["stats"].append((variant, "m2c_fail", None, None))
            continue
        cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, variant))
        clean, defsyms, summ, facts, text, nfix = check(
            u, vaddr, cpath, text, size, rodata, VARIANTS[variant][3], drive.fn_name(row["vaddr"]))
        res["tries"].append((variant, summ + ("+r%d" % nfix if nfix else "")))
        pos, al = closeness(facts, size)
        res["stats"].append((variant, "match" if clean else ("compiled" if al is not None else "error"),
                             pos, al))
        if al is not None and (res["best"] is None or al > res["best"]["aligned"]):
            res["best"] = {"variant": variant, "aligned": al, "positional": pos, "text": text}
        if clean and not res["match"]:
            res["match"] = {"variant": variant, "text": text, "defsyms": defsyms, "rodata": rodata}
        if clean and not try_all:
            break
    return res


def commit_match(res, args):
    """Write the C into re/match, re-verify it, then manifest + ledger. -> status string."""
    row, m = res["row"], res["match"]
    unit, vaddr_s = row["unit"], row["vaddr"]
    vaddr = int(vaddr_s, 16)
    cpath = drive.c_path(drive.RE_MATCH, unit, vaddr_s)
    if os.path.exists(cpath):
        return "skipped: %s appeared meanwhile" % drive.rel(cpath)
    header = ("/* %s %s  size %s  zero-agent sweep (tools/loop/sweep.py): m2c %s draft, %s */\n"
              % (unit, vaddr_s, row["size"], m["variant"], datetime.date.today().isoformat()))
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(header + m["text"])
    u = units.resolve(unit)
    argv = drive.oracle_argv(u, vaddr, cpath, int(row["size"]), m["rodata"], m["defsyms"])
    rc, facts, _t, _e = drive.run_oracle(argv)
    if not (rc == 0 and facts and facts.get("result") == "MATCH" and not facts.get("masked_words")
            and facts.get("bytes") == int(row["size"])):
        os.unlink(cpath)
        return "re-verify failed (%s); file removed" % ((facts or {}).get("result") or "ERROR")
    extra = []
    if m["rodata"]:
        extra += ["--rodata", m["rodata"]]
    syms = drive.syms_args(cpath)
    extra += syms
    for d in (drive.drop_table_defsyms(m["defsyms"], drive.table_syms()) if syms else m["defsyms"]):
        extra += ["--defsym", d]
    src = "%s/%s.c" % (unit, drive.fn_name(vaddr_s))
    drive.manifest_append(vaddr_s, u.manifest_unit, src, int(row["size"]), " ".join(extra))
    if not args.no_ledger and ledger.state(unit, vaddr_s) != "match":
        ledger.record("match", unit, vaddr_s, 0, "zero-agent sweep (m2c %s)" % m["variant"])
    return "MATCH"


REPORT_COLS = ["unit", "vaddr", "size", "group", "n_members", "status", "closeness", "positional",
               "best_variant", "tries"]


def print_variant_table(results, variants):
    """Per variant: drafts m2c produced, compiled (cc1 ok), clean matches, mean aligned
    closeness of its compiling drafts, and bodies where it was the closest."""
    print("%-16s %6s %8s %8s %6s %7s %5s" % ("variant", "tried", "drafted", "compiled", "match",
                                            "mean~", "best"))
    for v in variants:
        st = [s for x in results for s in x.get("stats", []) if s[0] == v]
        if not st:
            continue
        comp = [s for s in st if s[1] in ("compiled", "match")]
        best = sum(1 for x in results if x.get("best") and x["best"]["variant"] == v)
        mean = sum(s[3] for s in comp) / len(comp) if comp else 0.0
        print("%-16s %6d %8d %8d %6d %7.3f %5d" % (
            v, len(st), sum(1 for s in st if s[1] != "m2c_fail"), len(comp),
            sum(1 for s in st if s[1] == "match"), mean, best))


def load_queue_rows(paths, max_size, limit):
    current = ledger.load()
    rows, seen, skipped = [], set(), {"size": 0, "closed": 0, "c_file": 0}
    for p in paths:
        for r in drive.read_queue(p):
            key = (r["unit"], r["vaddr"])
            if key in seen:
                continue
            seen.add(key)
            if int(r["size"]) >= max_size:
                skipped["size"] += 1
                continue
            if ledger.state(r["unit"], r["vaddr"], current) != "open":
                skipped["closed"] += 1
                continue
            if os.path.exists(drive.c_path(drive.RE_MATCH, r["unit"], r["vaddr"])):
                skipped["c_file"] += 1
                continue
            rows.append(r)
    if limit:
        rows = rows[:limit]
    return rows, skipped


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--queue", action="append", required=True, help="queue TSV (repeatable)")
    ap.add_argument("--max-size", type=int, default=160, help="only bodies smaller than this (bytes)")
    ap.add_argument("--limit", type=int, default=0, help="first N eligible bodies (default all)")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 4))
    ap.add_argument("--work", help="scratch dir for drafts (default a new temp dir)")
    ap.add_argument("--report", help="per-body report TSV (default <work>/sweep_report.tsv)")
    ap.add_argument("--variants", default=DEFAULT_VARIANTS,
                    help="comma-separated drafts to try in order, of: %s (default %%(default)s)"
                    % ", ".join(VARIANTS))
    ap.add_argument("--try-all", action="store_true",
                    help="try every variant even after a match (measures them; the first match wins)")
    ap.add_argument("--best-dir", help="write each missed body's closest compiling draft here "
                    "(<unit>_<ADDR>.c, header names the variant and closeness)")
    ap.add_argument("--dry-run", action="store_true", help="list the bodies, run nothing")
    ap.add_argument("--no-replay", action="store_true", help="do not run twins.py replay afterwards")
    ap.add_argument("--no-ledger", action="store_true")
    args = ap.parse_args(argv)

    variants = [v.strip() for v in args.variants.split(",") if v.strip()]
    bad = [v for v in variants if v not in VARIANTS]
    if bad:
        ap.error("unknown variant(s) %s; known: %s" % (", ".join(bad), ", ".join(VARIANTS)))
    rows, skipped = load_queue_rows(args.queue, args.max_size, args.limit)
    print("%d bod(ies) to sweep (skipped: %d >= %d B, %d not open, %d with a C file)"
          % (len(rows), skipped["size"], args.max_size, skipped["closed"], skipped["c_file"]))
    if args.dry_run:
        for r in rows:
            print("%s %s %s B  %s" % (r["unit"], r["vaddr"], r["size"], r.get("group", "")))
        return 0
    workdir = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="ac1_sweep_")
    os.makedirs(workdir, exist_ok=True)
    report = args.report or os.path.join(workdir, "sweep_report.tsv")
    t0 = time.time()
    results = []
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(work, r, workdir, variants, args.try_all): r for r in rows}
        for i, f in enumerate(cf.as_completed(futs), 1):
            r = futs[f]
            try:
                res = f.result()
            except Exception as e:                  # noqa: BLE001 - one bad body must not stop the sweep
                res = {"row": r, "tries": [("-", "CRASH:%s" % e)], "match": None, "best": None,
                       "stats": []}
            res["status"] = commit_match(res, args) if res["match"] else "no match"
            results.append(res)
            print("[%3d/%d] %-7s %s %4s B  %-8s %s" % (
                i, len(rows), r["unit"], r["vaddr"], r["size"],
                "MATCH" if res["status"] == "MATCH" else "-", " | ".join("%s %s" % t for t in res["tries"])))
            sys.stdout.flush()
    hits = [x for x in results if x["status"] == "MATCH"]
    credited = sum(int(x["row"].get("n_members") or 1) for x in hits)
    with open(report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(REPORT_COLS) + "\n")
        for x in sorted(results, key=lambda x: (x["row"]["unit"], x["row"]["vaddr"])):
            r, b = x["row"], x.get("best")
            fh.write("\t".join([r["unit"], r["vaddr"], str(r["size"]), r.get("group", ""),
                                str(r.get("n_members", "")), x["status"],
                                "%.3f" % b["aligned"] if b else "", "%.3f" % b["positional"] if b else "",
                                b["variant"] if b else "",
                                " | ".join("%s %s" % t for t in x["tries"])]) + "\n")
    if args.best_dir:
        os.makedirs(args.best_dir, exist_ok=True)
        for x in results:
            r, b = x["row"], x.get("best")
            if b and x["status"] != "MATCH":
                p = os.path.join(args.best_dir, "%s_%s.c" % (r["unit"], r["vaddr"][2:].upper()))
                with open(p, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write("/* %s %s  size %s  sweep.py best draft: m2c %s, closeness %.3f aligned"
                             " / %.3f positional */\n%s" % (r["unit"], r["vaddr"], r["size"], b["variant"],
                                                            b["aligned"], b["positional"], b["text"]))
    print_variant_table(results, variants)
    print("sweep: %d/%d bodies matched (%.1f%%), %d function(s) credited with twins, %.0f s; report %s"
          % (len(hits), len(results), 100.0 * len(hits) / len(results) if results else 0,
             credited, time.time() - t0, report))
    if not args.work:
        for f in os.listdir(workdir):
            if f.endswith(".c"):
                os.unlink(os.path.join(workdir, f))
    if hits and not args.no_replay:
        import twins
        print("$ twins.py replay --scope programs")
        twins.main(["replay", "--scope", "programs", "-j", str(args.jobs)])
    return 0


if __name__ == "__main__":
    sys.exit(main())
