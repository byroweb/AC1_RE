#!/usr/bin/env python3
"""sweep.py - zero-agent sweep: m2c drafts straight into the oracle, no LLM in the loop.

    sweep.py --queue FILE [--queue FILE ...] [--max-size 160] [--limit 100] [-j 20]
             [--variants ctx,ctx-regvars,ctx-knr,neighbour] [--try-all]
             [--neighbour-min 0.5] [--neighbour-tries 1]
             [--work DIR] [--report FILE] [--dry-run] [--no-replay] [--no-ledger]

For each open body in the queue files (make_queues.py --bodies rows, or any
queue with unit/vaddr/size columns), in file order, skipping rows that are no
longer open in the ledger, that already have re/match/<unit>/func_<ADDR>.c, or
(for the m2c drafts; all of them without the neighbour variant) that are
--max-size bytes or larger:

  1. drafts, in --variants order until one matches: tools/match/ctx.py
     decompile() (symbolised asm + the shared-type context) with m2c
     --valid-syntax (`ctx`), then with --reg-vars saved (`ctx-regvars`), then
     `ctx` with m2c's guessed extern prototypes made unprototyped (`ctx-knr`);
     also known: ctx-no-andor, ctx-void, raw (no context); a draft
     gets `#include "ac1/ac1.h"` and `#include "m2c_compat.h"` (the M2C_* names);
     then `neighbour`: the C file of the most similar matched function (masked
     words), renamed, its func_/D_ names and --defsyms moved through the aligned
     relocation slots, plus `neighbour+imm` when only immediates differ (see
     Neighbours / neighbour_draft / imm_fixup; the report's neighbour column names
     it for every body);
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
import difflib
import os
import re
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

EXTERN_PROTO = re.compile(r"^([^\n;()]*?\b\w+)\([^\n;]*\);(\s*/\* extern \*/)$", re.M)


def knr_externs(text):
    """m2c guesses an `/* extern */` callee's parameter list from the registers set
    before the first call (`s32 SquareRoot0(s32, s32);` when a1 happened to be live),
    so a later call with fewer arguments is a cc1 error.  Declare those callees
    unprototyped instead (`s32 SquareRoot0();`): any call compiles, int-sized
    arguments are passed exactly as before."""
    return EXTERN_PROTO.sub(r"\1();\2", text)


# name -> (raw, m2c args, text fix-up or None); raw = ctx.decompile(raw=True): no
# context, no symbol rewrite.  Measured on the misses of the full sweep (README).
VARIANTS = {"ctx": (False, ["--valid-syntax"], None),
            "ctx-no-andor": (False, ["--valid-syntax", "--no-andor"], None),
            "ctx-knr": (False, ["--valid-syntax"], knr_externs),
            "ctx-regvars": (False, ["--valid-syntax", "--reg-vars", "saved"], None),
            "ctx-void": (False, ["--valid-syntax", "--void"], None),
            "raw": (True, ["--valid-syntax"], None)}
NEIGHBOUR = "neighbour"     # not an m2c draft: the nearest matched function's C (below)
DEFAULT_VARIANTS = "ctx,ctx-regvars,ctx-knr,neighbour"
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


def draft(row, variant):
    """C text of one draft, or raises ctx.CtxError."""
    raw, args, fixup = VARIANTS[variant]
    out = ctx.decompile(int(row["vaddr"], 16), int(row["size"]), row["unit"], raw=raw,
                        m2c_args=args)
    if not out.strip():
        raise ctx.CtxError("m2c printed nothing")
    if fixup:
        out = fixup(out)
    return PRELUDE + "\n" + fill_params(out, drive.fn_name(row["vaddr"]))


# ---------------------------------------------------------------------------
# neighbour: the most similar matched function's C, its names remapped
# ---------------------------------------------------------------------------
AUTO_NAME = re.compile(r"\b(func|D)_([0-9A-Fa-f]{8})\b")
IDENT = re.compile(r"\b[A-Za-z_]\w*\b")
NEIGHBOUR_HDR = "/* neighbour draft"
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
NEIGHBOUR_MIN = 0.5        # default --neighbour-min: lowest similarity worth an oracle run


def trigrams(m):
    return set(zip(m, m[1:], m[2:]))


class Neighbours:
    """Every original manifest row (its own func_<ADDR>.c) as (masked body, slots), one
    per distinct masked body; best(unit, vaddr, size) finds the most similar one.

    Similarity is difflib's ratio over the masked words (twins.mask_function: the
    relocated fields zeroed), computed for the 5 candidates of size 0.7x..1.43x with the
    highest masked-trigram Jaccard.  The whole corpus is read from the images, so a
    rebuild after a batch of matches costs a few seconds."""

    def __init__(self, manifest=drive.MANIFEST):
        import twins
        self.twins = twins
        self.corpus, seen = [], set()
        _h, rows, _crlf = drive.read_manifest(manifest)
        for r in rows:
            if not twins.is_original([r["vaddr"], r["unit"], r["src"]]):
                continue
            try:
                unit, va, size = units.norm(r["unit"] or "exe"), int(r["vaddr"], 16), int(r["size"])
                masked, slots = twins.mask_function(twins.func_words(unit, va, size), va)
            except (ValueError, units.UnitError, OSError):
                continue
            if masked in seen:
                continue
            seen.add(masked)
            self.corpus.append({"unit": unit, "vaddr": va, "size": size, "src": r["src"],
                                "extra": r["extra"], "masked": masked, "slots": slots,
                                "tri": trigrams(masked)})

    def ranked(self, unit, vaddr, size, k=1):
        """[(similarity, corpus entry, (masked, slots) of the body)] best first, at most k."""
        masked, slots = self.twins.mask_function(self.twins.func_words(unit, vaddr, size), vaddr)
        g = trigrams(masked)
        cands = []
        for c in self.corpus:
            if not (0.7 * size <= c["size"] <= 1.43 * size) or (c["unit"], c["vaddr"]) == (unit, vaddr):
                continue
            cands.append((len(g & c["tri"]) / (len(g | c["tri"]) or 1), c["unit"], c["vaddr"], c))
        cands.sort(key=lambda x: x[:3], reverse=True)
        out = []
        for _j, _u, _v, c in cands[:max(5, k)]:
            s = difflib.SequenceMatcher(None, c["masked"], masked, autojunk=False).ratio()
            out.append((s, c, (masked, slots)))
        out.sort(key=lambda x: -x[0])
        return out[:k]


def align_slots(nmasked, nslots, bmasked, bslots):
    """Relocation slots of the neighbour and the body paired through difflib's alignment
    of the masked words: equal blocks, and same-length replace blocks position by position
    (same slot kind only) -> (neighbour slots, body slots) as twins.map_addr takes them."""
    ni = {s[0]: s for s in nslots}
    bi = {s[0]: s for s in bslots}
    a, b = [], []
    sm = difflib.SequenceMatcher(None, nmasked, bmasked, autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal" or (tag == "replace" and i2 - i1 == j2 - j1):
            for k in range(i2 - i1):
                x, y = ni.get(i1 + k), bi.get(j1 + k)
                if x and y and x[1] == y[1]:
                    a.append(x)
                    b.append(y)
    return a, b


def neighbour_draft(row, sim, nb, body):
    """(C text, defsyms, extra args, rodata or None, notes) of the neighbour's C remapped
    onto the body: its function renamed, every func_/D_<ADDR> name moved by the delta of
    the aligned relocation slot that targets it (twins.map_addr), table names whose slot
    moved and its other --defsyms passed as --defsym at the moved address."""
    import twins
    unit, va = row["unit"], int(row["vaddr"], 16)
    with open(os.path.join(drive.RE_MATCH, nb["src"]), encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    na, ba = align_slots(nb["masked"], nb["slots"], body[0], body[1])
    notes = []

    def moved(addr):
        b, err = twins.map_addr(addr, na, ba, nb["unit"])
        if err:
            notes.append("0x%08X kept" % addr)
            return addr
        return b

    amap = {nb["vaddr"]: va}
    for m in AUTO_NAME.finditer(text):
        a = int(m.group(2), 16)
        if a not in amap:
            amap[a] = moved(a)
    # two names moved onto one address (a misaligned slot) would be a cc1 conflict:
    # keep those names where they were, so the draft still compiles as a starting point
    onto = {}
    for a, b in amap.items():
        onto.setdefault(b, []).append(a)
    for b, srcs in onto.items():
        if len(srcs) > 1:
            for a in srcs:
                if a != nb["vaddr"] and a != b:
                    amap[a] = a
                    notes.append("0x%08X kept (collides at 0x%08X)" % (a, b))
    text = AUTO_NAME.sub(lambda m: "%s_%08X" % (m.group(1), amap[int(m.group(2), 16)]), text)
    first, nl, rest = text.partition("\n")
    hdr = (NEIGHBOUR_HDR + " (tools/loop/sweep.py): %s 0x%08X %s, masked similarity %.3f */"
           % (nb["unit"], nb["vaddr"], nb["src"], sim))
    text = hdr + "\n" + (rest if first.startswith("/*") and first.rstrip().endswith("*/") else text)

    defs, rodata, other = twins.parse_extra(nb["extra"])
    code = COMMENT.sub(" ", text)
    defsyms = sorted({"%s_%08X=0x%08X" % (k, a, a) for k, a in
                      ((m.group(1), int(m.group(2), 16)) for m in AUTO_NAME.finditer(code))
                      if a != va})
    names = set(IDENT.findall(code))
    for n, a in defs:
        if not drive.AUTO_SYM.match(n) and n in names:
            defsyms.append("%s=0x%08X" % (n, moved(a)))
    tfiles = tuple(drive.AC1_SYMS) if drive.AC1_INCLUDE.search(text) else twins.row_tables(other)
    skip = {n for n, _a in defs}
    for n, b in twins.table_moves(twins.table_syms(tfiles), na, ba, skip):
        if n in names:
            defsyms.append("%s=0x%08X" % (n, b))
    if drive.AC1_INCLUDE.search(text):         # oracle_argv adds both tables for these
        rest_args, i = [], 0
        while i < len(other):
            if other[i] == "--syms":
                i += 2
                continue
            rest_args.append(other[i])
            i += 1
        other = rest_args
    nrod = None
    if rodata is not None:
        nrod = "0x%08X" % moved(rodata)
    return text, defsyms, other, nrod, notes


ASM_IMM = re.compile(r"(-?(?:0x[0-9a-f]+|\d+))(?=\(|$)")
C_LIT = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+)([uUlL]*)\b")
NO_IMM_FIX = {"lui", "j", "jal", "b", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez"}


def imm_fixup(text, diffs):
    """C text with its integer literals moved to retail's immediates, or None.

    Near-copies often differ only in constants (a loop bound, a clamp, a field offset):
    when every differing word has the same mnemonic and registers on both sides (diff
    rows are retail | ours), the literal of value `ours` (else `ours - 1`, a `<=`/`>`
    compare; else `-ours`, a subtraction) is replaced by the same form of retail's value.
    None when another kind of word differs or a literal would need two values."""
    head, nl, body = text.partition("\n")
    lits = {int(m.group(1), 0) for m in C_LIT.finditer(body)}
    use = {}
    for mark, _addr, a, b in diffs:
        if mark != "!":
            continue
        if drive.mnem(a) != drive.mnem(b) or drive.imm_norm(a) != drive.imm_norm(b) or drive.mnem(a) in NO_IMM_FIX:
            return None
        ma, mb = ASM_IMM.search(a), ASM_IMM.search(b)
        if not (ma and mb):
            return None
        want, have = int(ma.group(1), 0), int(mb.group(1), 0)
        # one form per diff: the first present in the C (exact, then compare, then negated)
        for v, new in ((have, want), (have - 1, want - 1), (-have, -want)):
            if v >= 2 and v in lits:
                if use.get(v, new) != new:
                    return None                 # one literal, two retail values
                use[v] = new
                break
    if not use:
        return None

    def sub(m):
        v = int(m.group(1), 0)
        if v not in use:
            return m.group(0)
        n = use[v]
        t = ("0x%X" if m.group(1).lower().startswith("0x") else "%d") % abs(n) + m.group(2)
        return "(-%s)" % t if n < 0 else t
    out = C_LIT.sub(sub, body)
    return head + nl + out if out != body else None


def oracle(u, vaddr, cpath, size, rodata, defsyms=(), extra=(), diffs=None):
    """(clean?, defsyms, facts-summary) with drive.py try's auto --defsym rerun.
    diffs: a list that gets the oracle's diff rows (drive.diff_rows) of a same-size MISMATCH."""
    defsyms = list(defsyms)
    argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms) + list(extra)
    rc, facts, _text, err = drive.run_oracle(argv)
    if facts and facts.get("unresolved_symbols"):
        auto = []
        for s in facts["unresolved_symbols"]:
            m = drive.AUTO_SYM.match(s)
            if m:
                auto.append("%s=0x%s" % (s, m.group(2).upper()))
        if auto:
            defsyms = defsyms + auto
            argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms) + list(extra)
            rc, facts, _text, err = drive.run_oracle(argv)
    result = facts["result"] if facts else "ERROR"
    masked = facts.get("masked_words", 0) if facts else 0
    clean = (rc == 0 and result == "MATCH" and masked == 0 and facts.get("bytes") == size)
    first = facts.get("first_diff_offset") if facts else None
    if diffs is not None and result == "MISMATCH" and facts.get("bytes") == size:
        diffs.extend(drive.diff_rows(_text))
    summ = result + ("(masked)" if result == "MATCH" and masked else "")
    if result == "MISMATCH" and first is not None:
        summ += "@0x%x/%s" % (first, facts.get("diff_words"))
    if result == "ERROR":
        msg = (facts or {}).get("error") or err.strip().splitlines()[-1:] or [""]
        summ += ":" + " ".join((msg if isinstance(msg, str) else msg[0]).split())[:80]
    return clean, defsyms, summ


def work(row, workdir, variants, try_all=False, neighbours=None, nb_min=NEIGHBOUR_MIN, nb_tries=1):
    """Try the variants in order on one body -> dict(result..., c_text, defsyms, variant).
    try_all: keep trying after the first clean one (to measure variants; the first wins).
    neighbours: a Neighbours index; the body's best neighbour goes to res["neighbour"]
    whether or not the `neighbour` variant is tried (it is only for similarity >= nb_min)."""
    unit, vaddr = row["unit"], int(row["vaddr"], 16)
    size = int(row["size"])
    u = units.resolve(unit)
    rodata = None
    if int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    res = {"row": row, "tries": [], "match": None, "neighbour": []}
    if neighbours is not None:
        res["neighbour"] = neighbours.ranked(unit, vaddr, size, max(1, nb_tries))
    for variant in variants:
        if res["match"] and not try_all:
            break
        if variant == NEIGHBOUR:
            for k, (sim, nb, body) in enumerate(res["neighbour"]):
                if sim < nb_min:
                    break
                name = variant if k == 0 else "%s%d" % (variant, k + 1)
                try:
                    text, defsyms, extra, nrod, _notes = neighbour_draft(row, sim, nb, body)
                except (OSError, ValueError) as e:
                    res["tries"].append((name, "DRAFT_FAIL:%s" % e))
                    continue
                cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, name))
                with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write(text)
                rod = rodata or nrod
                diffs = []
                clean, defsyms, summ = oracle(u, vaddr, cpath, size, rod, defsyms, extra, diffs)
                res["tries"].append((name, summ))
                fixed = None if clean else imm_fixup(text, diffs)
                if fixed:                           # constants only: one more try with retail's
                    name += "+imm"
                    text = fixed
                    cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, name))
                    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                        fh.write(text)
                    clean, defsyms, summ = oracle(u, vaddr, cpath, size, rod, defsyms, extra)
                    res["tries"].append((name, summ))
                if clean and not res["match"]:
                    res["match"] = {"variant": name, "text": text, "defsyms": defsyms, "rodata": rod,
                                    "extra": extra,
                                    "from": "%s draft of %s 0x%08X (%s, similarity %.3f)"
                                    % (name, nb["unit"], nb["vaddr"], nb["src"], sim),
                                    "why": "%s of %s 0x%08X, similarity %.3f"
                                    % (name, nb["unit"], nb["vaddr"], sim)}
                if clean:
                    break
            continue
        try:
            text = draft(row, variant)
        except ctx.CtxError as e:
            lines = str(e).strip().splitlines() or [""]
            res["tries"].append((variant, "M2C_FAIL:" + " ".join(lines[-1].split())[:80]))
            continue
        cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, variant))
        with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        clean, defsyms, summ = oracle(u, vaddr, cpath, size, rodata)
        res["tries"].append((variant, summ))
        if clean and not res["match"]:
            res["match"] = {"variant": variant, "text": text, "defsyms": defsyms, "rodata": rodata,
                            "extra": [], "from": "m2c %s draft" % variant, "why": "m2c %s" % variant}
    return res


def commit_match(res, args):
    """Write the C into re/match, re-verify it, then manifest + ledger. -> status string."""
    row, m = res["row"], res["match"]
    unit, vaddr_s = row["unit"], row["vaddr"]
    vaddr = int(vaddr_s, 16)
    cpath = drive.c_path(drive.RE_MATCH, unit, vaddr_s)
    if os.path.exists(cpath):
        return "skipped: %s appeared meanwhile" % drive.rel(cpath)
    header = ("/* %s %s  size %s  zero-agent sweep (tools/loop/sweep.py): %s, %s */\n"
              % (unit, vaddr_s, row["size"], m["from"], datetime.date.today().isoformat()))
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        body = m["text"]
        if body.startswith(NEIGHBOUR_HDR):        # the header says where it came from
            body = body.partition("\n")[2]
        fh.write(header + body)
    u = units.resolve(unit)
    argv = drive.oracle_argv(u, vaddr, cpath, int(row["size"]), m["rodata"], m["defsyms"]) + m["extra"]
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
    extra += m["extra"]
    src = "%s/%s.c" % (unit, drive.fn_name(vaddr_s))
    drive.manifest_append(vaddr_s, u.manifest_unit, src, int(row["size"]), " ".join(extra))
    if not args.no_ledger and ledger.state(unit, vaddr_s) != "match":
        ledger.record("match", unit, vaddr_s, 0, "zero-agent sweep (%s)" % m["why"])
    return "MATCH"


def fmt_neighbour(ranked):
    """Report column: `SIM UNIT VADDR SOURCE` of the best neighbour, or `-`."""
    if not ranked:
        return "-"
    sim, nb, _body = ranked[0]
    return "%.3f %s 0x%08X %s" % (sim, nb["unit"], nb["vaddr"], nb["src"])


def load_queue_rows(paths, max_size, limit, any_size=False):
    """Open rows of the queues; any_size keeps the bodies of max_size or more too
    (the neighbour variant has no size limit)."""
    current = ledger.load()
    rows, seen, skipped = [], set(), {"size": 0, "closed": 0, "c_file": 0}
    for p in paths:
        for r in drive.read_queue(p):
            key = (r["unit"], r["vaddr"])
            if key in seen:
                continue
            seen.add(key)
            if int(r["size"]) >= max_size and not any_size:
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
    ap.add_argument("--max-size", type=int, default=160,
                    help="m2c drafts only for bodies smaller than this (bytes); the neighbour"
                    " variant tries every size")
    ap.add_argument("--limit", type=int, default=0, help="first N eligible bodies (default all)")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 4))
    ap.add_argument("--work", help="scratch dir for drafts (default a new temp dir)")
    ap.add_argument("--report", help="per-body report TSV (default <work>/sweep_report.tsv)")
    ap.add_argument("--variants", default=DEFAULT_VARIANTS,
                    help="comma-separated drafts to try in order, of: %s (default %%(default)s)"
                    % ", ".join(list(VARIANTS) + [NEIGHBOUR]))
    ap.add_argument("--neighbour-min", type=float, default=NEIGHBOUR_MIN,
                    help="neighbour variant: lowest masked similarity tried (default %(default)s)")
    ap.add_argument("--neighbour-tries", type=int, default=1,
                    help="neighbour variant: try the best N neighbours (default %(default)s)")
    ap.add_argument("--try-all", action="store_true",
                    help="try every variant even after a match (measures them; the first match wins)")
    ap.add_argument("--dry-run", action="store_true", help="list the bodies, run nothing")
    ap.add_argument("--no-replay", action="store_true", help="do not run twins.py replay afterwards")
    ap.add_argument("--no-ledger", action="store_true")
    args = ap.parse_args(argv)

    variants = [v.strip() for v in args.variants.split(",") if v.strip()]
    bad = [v for v in variants if v not in VARIANTS and v != NEIGHBOUR]
    if bad:
        ap.error("unknown variant(s) %s; known: %s"
                 % (", ".join(bad), ", ".join(list(VARIANTS) + [NEIGHBOUR])))
    use_nb = NEIGHBOUR in variants
    rows, skipped = load_queue_rows(args.queue, args.max_size, args.limit, any_size=use_nb)
    print("%d bod(ies) to sweep (skipped: %d >= %d B, %d not open, %d with a C file)"
          % (len(rows), skipped["size"], args.max_size, skipped["closed"], skipped["c_file"]))
    t0 = time.time()
    neighbours = None
    if use_nb:
        neighbours = Neighbours()
        print("neighbour corpus: %d matched bodies (%.1f s)" % (len(neighbours.corpus), time.time() - t0))

    def row_variants(r):
        if int(r["size"]) < args.max_size:
            return variants
        return [NEIGHBOUR]
    if args.dry_run:
        for r in rows:
            nb = ""
            if neighbours:
                nb = fmt_neighbour(neighbours.ranked(r["unit"], int(r["vaddr"], 16), int(r["size"])))
            print("%s %s %s B  %s  %s" % (r["unit"], r["vaddr"], r["size"], r.get("group", ""), nb))
        return 0
    workdir = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="ac1_sweep_")
    os.makedirs(workdir, exist_ok=True)
    report = args.report or os.path.join(workdir, "sweep_report.tsv")
    results = []
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(work, r, workdir, row_variants(r), args.try_all, neighbours,
                          args.neighbour_min, args.neighbour_tries): r for r in rows}
        for i, f in enumerate(cf.as_completed(futs), 1):
            r = futs[f]
            try:
                res = f.result()
            except Exception as e:                  # noqa: BLE001 - one bad body must not stop the sweep
                res = {"row": r, "tries": [("-", "CRASH:%s" % e)], "match": None, "neighbour": []}
            res["status"] = commit_match(res, args) if res["match"] else "no match"
            results.append(res)
            if not res["tries"]:
                continue                            # nothing tried (no neighbour close enough)
            print("[%3d/%d] %-7s %s %4s B  %-8s %s" % (
                i, len(rows), r["unit"], r["vaddr"], r["size"],
                "MATCH" if res["status"] == "MATCH" else "-", " | ".join("%s %s" % t for t in res["tries"])))
            sys.stdout.flush()
    hits = [x for x in results if x["status"] == "MATCH"]
    credited = sum(int(x["row"].get("n_members") or 1) for x in hits)
    with open(report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\tvaddr\tsize\tgroup\tn_members\tstatus\ttries\tneighbour\n")
        for x in sorted(results, key=lambda x: (x["row"]["unit"], x["row"]["vaddr"])):
            r = x["row"]
            fh.write("\t".join([r["unit"], r["vaddr"], str(r["size"]), r.get("group", ""),
                                str(r.get("n_members", "")), x["status"],
                                " | ".join("%s %s" % t for t in x["tries"]) or "-",
                                fmt_neighbour(x.get("neighbour"))]) + "\n")
    tried = sum(1 for x in results if x["tries"])
    print("sweep: %d/%d bodies matched (%.1f%%; %d tried), %d function(s) credited with twins, %.0f s;"
          " report %s" % (len(hits), len(results), 100.0 * len(hits) / len(results) if results else 0,
                          tried, credited, time.time() - t0, report))
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
