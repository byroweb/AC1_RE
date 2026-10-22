#!/usr/bin/env python3
"""variants.py - one parallel batch of small C rewrites of a near miss, scored like the oracle.

    python tools/match/variants.py 0x<VADDR> <file.c> [--ovl N | --unit U] [--expect-size N]
           [--edits FILE ...] [--replace OLD NEW ...] [--levers LIST] [--pairs]
           [-j N] [--top N] [--out F | --apply] [--keep DIR] [--json PATH]
           [--rodata VA] [--defsym NAME=ADDR ...] [--fn NAME] [--no-oracle]

The "sweep" step of the match-function loop: instead of one edit per oracle run, write
the candidate edits down once and get every variant's distance from the retail bytes
in one go.  Each variant is compiled with exactly the oracle's pipeline, in process
(tools/loop/permute.py: cpp, cc1 2.7.2, rodata align fix, maspsx 2.56, as, then
match_fn's relocation resolution with the --syms tables, func_<ADDR>/D_<ADDR> as their
own addresses, --rodata), -j at a time, and ranked by

  score      differing words at the same offset + 2 per word of size difference
             (permute.word_score; 0 = the same bytes)
  unaligned  retail words difflib cannot pair, in order, with an equal word of ours
             (stays small when one extra instruction shifts everything after it)

Every score-0 variant then goes through match_fn.py itself (sweep.oracle: --expect-size,
unit flags, --syms, auto --defsym rerun); only a clean MATCH there (exit 0, no masked
word, size equal) counts, and only then is the exit status 0.  The input file is not
touched unless --apply is given.

Where the variants come from (any mix, all applied to the input text):

  --edits FILE   blocks of replacements, diff-like:

                     # a comment (outside the - / + lines)
                     ## swap the two stores
                     -    o->y += 32;
                     -    o->w += 16;
                     +    o->w += 16;
                     +    o->y += 32;
                     ## w is signed
                     -    u16 w;
                     +    s16 w;

                 `## label` starts a variant; a run of `-` lines then a run of `+` lines is
                 one replacement (the text after the one-character prefix, verbatim); a
                 variant may hold several.  The old text must occur exactly once: first
                 as written, else with runs of whitespace matching any whitespace.  No `+`
                 lines deletes the old text.
  --replace OLD NEW   one more variant with one replacement (`\\n`, `\\t` unescaped).
  --levers LIST  tools/loop/levers.py rewrites of the function (index, bytes, args, ptype,
                 etype, const, swap, commute, and imm / cmp from the input's diff with
                 retail; `all`), one variant per site.
  --pairs        then every pair of the --edits/--replace variants that improved on the
                 input (best --pair-top of them), both applied.

Output: the input's score, then the --top best variants (status `same` = compiles to
the input's exact bytes, `NOEDIT` = an edit did not apply, `ERROR` = cc1/as failed,
with its first error line); --out F writes the best
variant's text (if it beats the input) to F, --apply writes it over the input file,
--keep DIR keeps every variant as DIR/vNNN.c with DIR/variants.tsv, --json PATH all rows.
Exit 0 = some variant is a clean oracle MATCH, 1 = none, 2 = bad arguments / edits.
"""
import argparse
import concurrent.futures as cf
import difflib
import hashlib
import json
import os
import re
import shutil
import struct
import sys
import tempfile
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
LOOP_DIR = os.path.join(REPO, "tools", "loop")
sys.path.insert(0, HERE)
sys.path.insert(0, LOOP_DIR)
import match_fn  # noqa: E402
import permute   # noqa: E402

UNIT_RE = re.compile(r"^(exe|ovl20[1-4]|fdat\d{3})$")
EXE_RANGE = (0x80011000, 0x8004ADA0)


class EditError(Exception):
    pass


# ---------------------------------------------------------------------------
# edits
# ---------------------------------------------------------------------------
def parse_edits(text, source="edits"):
    """`## label` blocks of `-old` / `+new` line runs -> [(label, [(old, new), ...])]."""
    out = []
    cur = None
    old, new = None, None

    def flush_hunk():
        nonlocal old, new
        if old is not None:
            if not old:
                raise EditError("%s: variant %r: a replacement needs at least one `-` line"
                                % (source, cur[0]))
            cur[1].append(("\n".join(old), "\n".join(new or [])))
        old, new = None, None

    for n, line in enumerate(text.splitlines(), 1):
        if line.startswith("##"):
            flush_hunk()
            if cur and not cur[1]:
                raise EditError("%s:%d: variant %r has no replacement" % (source, n, cur[0]))
            cur = (line[2:].strip() or "variant %d" % (len(out) + 1), [])
            out.append(cur)
        elif line.startswith("-") or line.startswith("+"):
            if cur is None:
                raise EditError("%s:%d: `%s` line before the first `## label`" % (source, n, line[0]))
            if line[0] == "-":
                if new is not None:          # a `-` after `+` lines starts the next replacement
                    flush_hunk()
                old = (old or []) + [line[1:]]
            else:
                if old is None:
                    raise EditError("%s:%d: `+` line without `-` lines before it" % (source, n))
                new = (new or []) + [line[1:]]
        elif line.startswith("#") or not line.strip():
            continue
        else:
            raise EditError("%s:%d: want `## label`, `-old`, `+new`, `# comment` or blank: %r"
                            % (source, n, line[:60]))
    flush_hunk()
    if cur and not cur[1]:
        raise EditError("%s: variant %r has no replacement" % (source, cur[0]))
    return out


def find_once(text, old):
    """(start, end) of the only occurrence of old in text: exact, else whitespace-tolerant."""
    n = text.count(old)
    if n == 1:
        i = text.index(old)
        return i, i + len(old)
    if n > 1:
        raise EditError("found %d times: %r" % (n, old.strip()[:50]))
    toks = old.split()
    if not toks:
        raise EditError("empty old text")
    pat = re.compile(r"\s+".join(re.escape(t) for t in toks))
    ms = list(pat.finditer(text))
    if len(ms) != 1:
        raise EditError("%s: %r" % ("not found" if not ms else "found %d times" % len(ms), old.strip()[:50]))
    return ms[0].start(), ms[0].end()


def apply_hunks(text, hunks):
    for old, new in hunks:
        a, b = find_once(text, old)
        text = text[:a] + new + text[b:]
    return text


def unescape(s):
    return s.replace("\\n", "\n").replace("\\t", "\t")


# ---------------------------------------------------------------------------
# scoring
# ---------------------------------------------------------------------------
def words(b):
    return list(struct.unpack_from("<%dI" % (len(b) // 4), b))


def unaligned(ours, retail):
    """Retail words difflib does not pair with an equal word of ours."""
    a, b = words(ours), words(retail)
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    return len(b) - sum(m.size for m in sm.get_matching_blocks())


def build(meta, syms, text, inc_dirs):
    """Draft C text -> (function bytes, unresolved names, None) or (None, None, error line)."""
    work = tempfile.mkdtemp(prefix="ac1_var_")
    try:
        c = os.path.join(work, "fn.c")
        with open(c, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        permute.preprocess(meta["tools"], c, os.path.join(work, "fn.i"), inc_dirs)
        fn_bytes, unres = permute.resolve_object(
            permute.build_object(meta, os.path.join(work, "fn.i"), work, meta["tools"]), None, meta, syms)
        return fn_bytes, unres, None
    except (RuntimeError, match_fn.ToolError, OSError, ValueError, struct.error) as e:
        msg = [ln for ln in str(e).replace("\r", "").splitlines() if ln.strip()]
        err = next((ln for ln in msg if "error" in ln.lower()), msg[0] if msg else type(e).__name__)
        return None, None, " ".join(err.split())[:120]
    finally:
        shutil.rmtree(work, ignore_errors=True)


def score_row(row, meta, syms, retail, inc_dirs):
    ours, unres, err = build(meta, syms, row["text"], inc_dirs)
    if ours is None:
        row.update(status="ERROR", error=err)
        return row
    row.update(status="built", score=permute.word_score(ours, retail), unaligned=unaligned(ours, retail),
               size=len(ours), unresolved=unres or [], code=hashlib.sha1(ours).hexdigest()[:10])
    return row


def sort_key(r):
    return (r.get("score") is None, r.get("score") or 0, r.get("unaligned") or 0, r["i"])


# ---------------------------------------------------------------------------
def resolve_unit(args):
    if args.unit:
        import units
        return units.norm(args.unit)
    if args.ovl:
        return "ovl%d" % args.ovl
    parent = os.path.basename(os.path.dirname(os.path.abspath(args.cfile)))
    if UNIT_RE.match(parent):
        return parent
    if EXE_RANGE[0] <= args.vaddr < EXE_RANGE[1]:
        return "exe"
    raise SystemExit("variants.py: overlay address 0x%08X: pass --ovl N or --unit U" % args.vaddr)


def make_meta(args, unit, tools):
    import drive
    vs = "0x%08X" % args.vaddr
    row = drive.load_rows().get((unit, vs))
    size = args.expect_size or (int(row["size"]) if row else None)
    if not size:
        raise SystemExit("variants.py: %s %s not in functions.tsv: pass --expect-size" % (unit, vs))
    rodata = args.rodata
    if rodata is None and row and int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    return {"unit": unit, "vaddr": vs, "fn": args.fn, "size": size, "rodata": rodata,
            "syms": permute.SYMS_TABLES if drive.uses_ac1_h(args.cfile) else [],
            "defsyms": list(args.defsym or []), "tools": tools}


def collect(args, base_text, fn, facts=None):
    """-> [(label, kind, hunks or None, text)] in input order (no scoring).  facts: the
    input's levers.diff_facts, for the diff levers (imm, cmp)."""
    out = []
    for path in args.edits or []:
        with open(path, encoding="utf-8") as fh:
            for label, hunks in parse_edits(fh.read(), os.path.basename(path)):
                out.append((label, "edit", hunks, None))
    for k, (old, new) in enumerate(args.replace or [], 1):
        out.append(("replace %d: %s" % (k, " ".join(unescape(old).split())[:40]), "edit",
                    [(unescape(old), unescape(new))], None))
    if args.levers:
        import levers
        names = levers.DEFAULT_LEVERS if args.levers == "all" else args.levers
        unknown = [n for n in names.split(",") if n.strip() and n.strip() not in levers.ALL_LEVERS]
        if unknown:
            raise SystemExit("variants.py: unknown lever(s) %s (have %s)" % (",".join(unknown),
                                                                             levers.DEFAULT_LEVERS))
        for label, text in levers.variants(base_text, fn, names, facts):
            out.append((label, "lever", None, text))
    return out


def materialise(cands, base_text, start=1):
    """Apply edits; drop texts seen before.  -> rows."""
    rows, seen = [], {hashlib.sha1(base_text.encode()).digest()}
    for i, (label, kind, hunks, text) in enumerate(cands, start):
        row = {"i": i, "label": label, "kind": kind, "hunks": hunks}
        if text is None:
            try:
                text = apply_hunks(base_text, hunks)
            except EditError as e:
                row.update(status="NOEDIT", error=str(e))
                rows.append(row)
                continue
        h = hashlib.sha1(text.encode()).digest()
        if h in seen:
            continue
        seen.add(h)
        row["text"] = text
        rows.append(row)
    return rows


def run_batch(rows, meta, syms, retail, inc_dirs, jobs, progress):
    todo = [r for r in rows if "text" in r and "status" not in r]
    done = [0]
    lock = threading.Lock()

    def one(r):
        score_row(r, meta, syms, retail, inc_dirs)
        with lock:
            done[0] += 1
            if progress and (done[0] % 25 == 0 or done[0] == len(todo)):
                sys.stderr.write("  %d/%d compiled\r" % (done[0], len(todo)))
                sys.stderr.flush()

    with cf.ThreadPoolExecutor(max_workers=max(1, jobs)) as ex:
        list(ex.map(one, todo))
    if progress and todo:
        sys.stderr.write("\n")
    return len(todo)


def confirm(rows, args, meta, unit, work):
    """match_fn.py on every score-0 row; sets row['oracle'] and returns the clean ones."""
    import sweep
    import units
    u = units.resolve(unit)
    inc = ["-I", os.path.dirname(os.path.abspath(args.cfile))]
    clean_rows = []
    for r in rows:
        if r.get("score") != 0:
            continue
        d = os.path.join(work, "oracle_v%03d" % r["i"])
        os.makedirs(d, exist_ok=True)
        p = os.path.join(d, os.path.basename(args.cfile))
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(r["text"])
        clean, defsyms, summ, _facts = sweep.oracle(u, args.vaddr, p, meta["size"], meta["rodata"],
                                                    defsyms=meta["defsyms"], extra=inc)
        r["oracle"] = summ
        r["defsyms"] = defsyms
        if clean:
            r["status"] = "MATCH"
            clean_rows.append(r)
    return clean_rows


def fmt(r):
    if r.get("score") is None:
        return "%4s  %-6s %5s %5s  %-34s %s" % (r["i"], r.get("status", ""), "", "", r["label"][:34],
                                                 r.get("error", ""))
    extra = r.get("oracle") or ""
    if r.get("unresolved"):
        extra += (" " if extra else "") + "unresolved " + ",".join(r["unresolved"][:4])
    return "%4s  %-6s %5d %5d  %-34s %s" % (r["i"], r["status"], r["score"], r["unaligned"],
                                             r["label"][:34], extra)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("vaddr", type=match_fn.parse_int, help="retail address of the function (0x...)")
    ap.add_argument("cfile", help="the near-miss C file (not modified unless --apply)")
    ap.add_argument("--ovl", type=int, help="overlay 201..204")
    ap.add_argument("--unit", help="exe | ovl201..ovl204 | fdatNNN (default: --ovl, the file's directory, "
                    "or exe by address)")
    ap.add_argument("--expect-size", type=match_fn.parse_int, help="retail size (default: functions.tsv)")
    ap.add_argument("--rodata", help="VA of the jump tables (default: functions.tsv jt_addrs)")
    ap.add_argument("--defsym", action="append", metavar="NAME=ADDR", help="as match_fn.py (repeatable)")
    ap.add_argument("--fn", help="function name in the object (default: the only one; levers use "
                    "func_<VADDR>)")
    ap.add_argument("--edits", action="append", metavar="FILE", help="`## label` / `-old` / `+new` blocks")
    ap.add_argument("--replace", nargs=2, action="append", metavar=("OLD", "NEW"), help="one variant")
    ap.add_argument("--levers", help="levers.py names, comma-separated, or `all`")
    ap.add_argument("--pairs", action="store_true", help="also combine improving edit variants two by two")
    ap.add_argument("--pair-top", type=int, default=12, help="edit variants --pairs combines (default 12)")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--top", type=int, default=15, help="rows printed (default 15; 0 = all)")
    ap.add_argument("--out", metavar="F", help="write the best variant here if it beats the input")
    ap.add_argument("--apply", action="store_true", help="write the best variant over the input if it "
                    "beats it")
    ap.add_argument("--keep", metavar="DIR", help="keep every variant as DIR/vNNN.c + DIR/variants.tsv")
    ap.add_argument("--json", metavar="PATH", help="all rows as JSON (`-` = stdout)")
    ap.add_argument("--no-oracle", action="store_true", help="skip the match_fn.py check of score-0 rows")
    args = ap.parse_args(argv)
    if not (args.edits or args.replace or args.levers):
        ap.error("give --edits, --replace and/or --levers")

    unit = resolve_unit(args)
    tools = permute.toolchain()
    meta = make_meta(args, unit, tools)
    syms = permute.symtab(meta)
    retail = permute.retail_bytes(unit, args.vaddr, meta["size"])
    fn = args.fn or "func_%08X" % args.vaddr
    with open(args.cfile, encoding="utf-8") as fh:
        base_text = fh.read()
    inc_dirs = [os.path.dirname(os.path.abspath(args.cfile))]
    facts = None
    if args.levers:
        import levers
        if levers.wants_facts(levers.DEFAULT_LEVERS if args.levers == "all" else args.levers):
            ours = build(meta, syms, base_text, inc_dirs)[0]
            facts = permute.ins_facts(meta, ours, retail) if ours is not None else None
    try:
        cands = collect(args, base_text, fn, facts)
    except (EditError, OSError) as e:
        sys.stderr.write("variants.py: %s\n" % e)
        return 2
    if not cands:
        sys.stderr.write("variants.py: no variants (levers found no site in %s?)\n" % fn)
        return 2

    t0 = time.time()
    base = {"i": 0, "label": "(input)", "kind": "input", "hunks": None, "text": base_text}
    rows = materialise(cands, base_text)
    n_compiles = run_batch([base] + rows, meta, syms, retail, inc_dirs, args.jobs, True)

    if args.pairs:
        ok = [r for r in rows if r["kind"] == "edit" and r.get("score") is not None
              and (base.get("score") is None or r["score"] < base["score"])]
        ok.sort(key=sort_key)
        ok = ok[:args.pair_top]
        pairs = [("%s + %s" % (a["label"], b["label"]), "pair", a["hunks"] + b["hunks"], None)
                 for k, a in enumerate(ok) for b in ok[k + 1:]]
        if pairs:
            prs = [r for r in materialise(pairs, base_text, len(cands) + 1) if "text" in r]
            n_compiles += run_batch(prs, meta, syms, retail, inc_dirs, args.jobs, True)
            rows += prs
    wall = time.time() - t0
    for r in rows:
        if r.get("status") == "built" and r["code"] == base.get("code"):
            r["status"] = "same"           # compiles to the input's bytes: a no-op for codegen

    work =os.path.abspath(args.keep) if args.keep else tempfile.mkdtemp(prefix="ac1_variants_")
    os.makedirs(work, exist_ok=True)
    try:
        clean = [] if args.no_oracle else confirm([base] + rows, args, meta, unit, work)
        if args.keep:
            write_keep(work, [base] + rows)
    finally:
        if not args.keep:
            shutil.rmtree(work, ignore_errors=True)

    ranked = sorted(rows, key=sort_key)
    print("%s %s %s  %d B  %d variant(s), %d compile(s) in %.1f s at -j %d"
          % (unit, meta["vaddr"], os.path.basename(args.cfile), meta["size"], len(rows), n_compiles, wall,
             args.jobs))
    print("%4s  %-6s %5s %5s  %s" % ("#", "status", "score", "unal", "variant"))
    print(fmt(base))
    for r in ranked[:args.top or None]:
        print(fmt(r))
    hidden = len(ranked) - (args.top or len(ranked))
    if hidden > 0:
        print("  ... %d more (--top 0 for all)" % hidden)

    best = ranked[0] if ranked and ranked[0].get("score") is not None else None
    better = best is not None and (base.get("score") is None or sort_key(best) < sort_key(base))
    if clean:
        best, better = min(clean, key=sort_key), True
    if better:
        for dest in ([args.out] if args.out else []) + ([args.cfile] if args.apply else []):
            with open(dest, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(best["text"])
            print("wrote #%d (%s) to %s" % (best["i"], best["label"], dest))
    elif args.out or args.apply:
        print("no variant beats the input: nothing written")
    if args.json:
        data = [{k: v for k, v in r.items() if k not in ("text", "hunks")} for r in [base] + ranked]
        s = json.dumps(data, indent=1)
        if args.json == "-":
            print(s)
        else:
            with open(args.json, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(s + "\n")
    if clean:
        print("MATCH %s: #%s" % (meta["vaddr"], ", #".join(str(r["i"]) for r in sorted(clean, key=sort_key))))
        return 0
    print("NO MATCH %s: best score %s (input %s)" % (meta["vaddr"], best["score"] if best else "-",
                                                     base.get("score", "ERROR")))
    return 1


def write_keep(d, rows):
    with open(os.path.join(d, "variants.tsv"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("i\tstatus\tscore\tunaligned\tsize\tcode\toracle\tlabel\terror\n")
        for r in rows:
            if "text" in r:
                with open(os.path.join(d, "v%03d.c" % r["i"]), "w", encoding="utf-8", newline="\n") as c:
                    c.write(r["text"])
            fh.write("\t".join("" if r.get(k) is None else str(r.get(k)) for k in
                               ("i", "status", "score", "unaligned", "size", "code", "oracle", "label", "error"))
                     + "\n")


if __name__ == "__main__":
    sys.exit(main())
