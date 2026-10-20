#!/usr/bin/env python3
"""sweep.py - zero-agent sweep: m2c drafts straight into the oracle, no LLM in the loop.

    sweep.py --queue FILE [--queue FILE ...] [--max-size 160] [--limit 100] [-j 20]
             [--work DIR] [--report FILE] [--dry-run] [--no-replay] [--no-ledger]

For each open body in the queue files (make_queues.py --bodies rows, or any
queue with unit/vaddr/size columns), in file order, skipping rows that are no
longer open in the ledger, that already have re/match/<unit>/func_<ADDR>.c, or
that are --max-size bytes or larger:

  1. drafts: tools/match/ctx.py decompile() (symbolised asm + the shared-type
     context) with m2c --valid-syntax, then the same with --no-andor; a draft
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
results, first diff) goes to --report (default <work>/sweep_report.tsv).
Nothing is recorded for a body that did not match: a miss here says nothing
about how hard the function is for a matcher.
"""
import argparse
import concurrent.futures as cf
import datetime
import os
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

VARIANTS = [("ctx", ["--valid-syntax"]),
            ("ctx-no-andor", ["--valid-syntax", "--no-andor"])]
PRELUDE = '#include "ac1/ac1.h"\n#include "m2c_compat.h"\n'


def draft(row, variant):
    """C text of one draft, or raises ctx.CtxError."""
    _name, args = variant
    out = ctx.decompile(int(row["vaddr"], 16), int(row["size"]), row["unit"], m2c_args=args)
    if not out.strip():
        raise ctx.CtxError("m2c printed nothing")
    return PRELUDE + "\n" + out


def oracle(u, vaddr, cpath, size, rodata):
    """(clean?, defsyms, facts-summary) with drive.py try's auto --defsym rerun."""
    defsyms = []
    argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms)
    rc, facts, _text, err = drive.run_oracle(argv)
    if facts and facts.get("unresolved_symbols"):
        auto = []
        for s in facts["unresolved_symbols"]:
            m = drive.AUTO_SYM.match(s)
            if m:
                auto.append("%s=0x%s" % (s, m.group(2).upper()))
        if auto:
            defsyms = auto
            argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms)
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
        summ += ":" + (msg if isinstance(msg, str) else msg[0])[:80].replace("\t", " ")
    return clean, defsyms, summ


def work(row, workdir):
    """Try every variant on one body -> dict(result..., c_text, defsyms, variant)."""
    unit, vaddr = row["unit"], int(row["vaddr"], 16)
    size = int(row["size"])
    u = units.resolve(unit)
    rodata = None
    if int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    res = {"row": row, "tries": [], "match": None}
    for variant in VARIANTS:
        try:
            text = draft(row, variant)
        except ctx.CtxError as e:
            res["tries"].append((variant[0], "M2C_FAIL:" + str(e).strip().splitlines()[-1][:80]))
            continue
        cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, variant[0]))
        with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        clean, defsyms, summ = oracle(u, vaddr, cpath, size, rodata)
        res["tries"].append((variant[0], summ))
        if clean:
            res["match"] = {"variant": variant[0], "text": text, "defsyms": defsyms, "rodata": rodata}
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
    ap.add_argument("--dry-run", action="store_true", help="list the bodies, run nothing")
    ap.add_argument("--no-replay", action="store_true", help="do not run twins.py replay afterwards")
    ap.add_argument("--no-ledger", action="store_true")
    args = ap.parse_args(argv)

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
        futs = {ex.submit(work, r, workdir): r for r in rows}
        for i, f in enumerate(cf.as_completed(futs), 1):
            r = futs[f]
            try:
                res = f.result()
            except Exception as e:                  # noqa: BLE001 - one bad body must not stop the sweep
                res = {"row": r, "tries": [("-", "CRASH:%s" % e)], "match": None}
            res["status"] = commit_match(res, args) if res["match"] else "no match"
            results.append(res)
            print("[%3d/%d] %-7s %s %4s B  %-8s %s" % (
                i, len(rows), r["unit"], r["vaddr"], r["size"],
                "MATCH" if res["status"] == "MATCH" else "-", " | ".join("%s %s" % t for t in res["tries"])))
            sys.stdout.flush()
    hits = [x for x in results if x["status"] == "MATCH"]
    credited = sum(int(x["row"].get("n_members") or 1) for x in hits)
    with open(report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\tvaddr\tsize\tgroup\tn_members\tstatus\ttries\n")
        for x in sorted(results, key=lambda x: (x["row"]["unit"], x["row"]["vaddr"])):
            r = x["row"]
            fh.write("\t".join([r["unit"], r["vaddr"], str(r["size"]), r.get("group", ""),
                                str(r.get("n_members", "")), x["status"],
                                " | ".join("%s %s" % t for t in x["tries"])]) + "\n")
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
