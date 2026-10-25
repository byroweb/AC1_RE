#!/usr/bin/env python3
"""sweep.py - zero-agent sweep: m2c drafts straight into the oracle, no LLM in the loop.

    sweep.py --queue FILE [--queue FILE ...] [--max-size 160] [--limit 100] [-j 20]
             [--variants ctx,ctx-regvars,ctx-knr,...,neighbour] [--try-all]
             [--neighbour-min 0.5] [--neighbour-op-min 0.6] [--neighbour-tries 1]
             [--work DIR] [--report FILE] [--best-dir DIR] [--dry-run] [--no-replay]
             [--no-ledger]

For each open body in the queue files (make_queues.py --bodies rows, or any
queue with unit/vaddr/size columns), in file order, skipping rows that are no
longer open in the ledger, that already have re/match/<unit>/func_<ADDR>.c, or
(for the m2c drafts; all of them without the neighbour variant) that are
--max-size bytes or larger:

  1. drafts, in --variants order until one matches: tools/match/ctx.py
     decompile() (symbolised asm + the shared-type context) with m2c
     --valid-syntax (`ctx`), then with --reg-vars saved (`ctx-regvars`), then
     `ctx` with m2c's guessed extern prototypes made unprototyped (`ctx-knr`);
     then the tools/loop/m2cfix.py fix-ups `ctx-spframe` (all spNN as fields of one
     frame union at their offsets), `ctx-fix-frame` / `ctx-fix` (knr + spframe or
     spdecl + voidcast, then up to 6 rounds of m2cfix.repair() on the lines cc1
     rejects); also known: ctx-no-andor, ctx-void, raw (no context), ctx-spdecl,
     ctx-voidcast, ctx-fix-regvars; m2c runs once per distinct argument list; a draft
     gets `#include "ac1/ac1.h"` and `#include "m2c_compat.h"` (the M2C_* names);
     then `neighbour`: the C file of the most similar matched function (masked
     words), renamed, its func_/D_ names and --defsyms moved through the aligned
     relocation slots, plus `neighbour+imm` when only immediates differ (see
     Neighbours / neighbour_draft / imm_fixup; the report's neighbour column names
     it for every body); then `neighbour-op`, the backup for bodies whose best
     masked neighbour is under --neighbour-min: the same draft from the nearest
     function by opcodes only (registers and immediates ignored), at or above
     --neighbour-op-min;
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
import difflib
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
import heartbeat  # noqa: E402
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
NEIGHBOUR = "neighbour"     # not an m2c draft: the nearest matched function's C (below)
NEIGHBOUR_OP = "neighbour-op"   # the backup: nearest by opcodes only, when no masked neighbour is close
DEFAULT_VARIANTS = "ctx,ctx-regvars,ctx-knr,ctx-spframe,ctx-fix-frame,ctx-fix,neighbour,neighbour-op"
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
    out = m2cfix.unaligned_copies(out)
    for f in fixups:
        out = f(out, name, row)
    if SS in args:                       # the template's types are used; drop the struct
        out = m2cfix.stack_template(out)[0]
    return PRELUDE + "\n" + fill_params(out, name)


# ---------------------------------------------------------------------------
# neighbour: the most similar matched function's C, its names remapped
# ---------------------------------------------------------------------------
AUTO_NAME = re.compile(r"\b(func|D)_([0-9A-Fa-f]{8})\b")
IDENT = re.compile(r"\b[A-Za-z_]\w*\b")
NEIGHBOUR_HDR = "/* neighbour draft"
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
NEIGHBOUR_MIN = 0.5        # default --neighbour-min: lowest similarity worth an oracle run
NEIGHBOUR_OP_MIN = 0.6     # default --neighbour-op-min: the same for the opcode-only backup


def trigrams(m):
    return set(zip(m, m[1:], m[2:]))


def opkey(w):
    """The operation of one MIPS word with registers and immediates dropped: the
    primary opcode, plus funct for SPECIAL, rt for REGIMM and the command for COP0..3
    (the GTE commands of COP2 stay apart).  0 (nop) is its own key."""
    if w == 0:
        return -1
    op = w >> 26
    if op == 0:
        return 0x100 | (w & 0x3F)
    if op == 1:
        return 0x200 | ((w >> 16) & 0x1F)
    if 0x10 <= op <= 0x13:
        if (w >> 25) & 1:                         # CO bit: a command (a GTE op for COP2)
            return 0x800 | (op << 6) | (w & 0x3F)
        return 0x400 | (op << 6) | ((w >> 21) & 0x1F)   # mfc / mtc / cfc / ctc / bc
    return op


class Neighbours:
    """Every original manifest row (its own func_<ADDR>.c) as (masked body, slots), one
    per distinct masked body; ranked(unit, vaddr, size) finds the most similar ones.

    mode "masked" (the default, and what the `neighbour` draft uses): difflib's ratio over
    the masked words (twins.mask_function: the relocated fields zeroed), computed for the
    5 candidates of size 0.7x..1.43x with the highest masked-trigram Jaccard.
    mode "opcode" (the backup, coddog-style): the same over opkey() sequences, so register
    choice and every immediate are ignored, for sizes 0.5x..2x.  It finds the relatives
    the masked words miss: useless as a draft more often, but a reference for how this
    codebase wrote the same shape of code.
    The whole corpus is read from the images, so a rebuild after a batch of matches costs
    a few seconds."""

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
            ops = tuple(opkey(w) for w in masked)
            self.corpus.append({"unit": unit, "vaddr": va, "size": size, "src": r["src"],
                                "extra": r["extra"], "masked": masked, "slots": slots,
                                "tri": trigrams(masked), "ops": ops, "otri": trigrams(ops)})

    def ranked(self, unit, vaddr, size, k=1, mode="masked"):
        """[(similarity, corpus entry, (masked, slots) of the body)] best first, at most k."""
        unit = units.norm(unit)
        masked, slots = self.twins.mask_function(self.twins.func_words(unit, vaddr, size), vaddr)
        if mode == "opcode":
            seq, key, tri, lo, hi = tuple(opkey(w) for w in masked), "ops", "otri", 0.5, 2.0
        else:
            seq, key, tri, lo, hi = masked, "masked", "tri", 0.7, 1.43
        g = trigrams(seq)
        cands = []
        for c in self.corpus:
            if not (lo * size <= c["size"] <= hi * size) or (c["unit"], c["vaddr"]) == (unit, vaddr):
                continue
            cands.append((len(g & c[tri]) / (len(g | c[tri]) or 1), c["unit"], c["vaddr"], c))
        cands.sort(key=lambda x: x[:3], reverse=True)
        out = []
        for _j, _u, _v, c in cands[:max(5, k)]:
            s = difflib.SequenceMatcher(None, c[key], seq, autojunk=False).ratio()
            out.append((s, c, (masked, slots)))
        out.sort(key=lambda x: -x[0])
        return out[:k]

    def references(self, unit, vaddr, size, k=3, masked_min=NEIGHBOUR_MIN):
        """Up to k matched functions to read as reference C: the masked neighbours at or
        above masked_min first, then opcode-only neighbours to fill the rest (each body at
        most once) -> [(similarity, mode, corpus entry)]."""
        out, seen = [], set()
        for s, c, _b in self.ranked(unit, vaddr, size, k):
            if s >= masked_min:
                out.append((s, "masked", c))
                seen.add((c["unit"], c["vaddr"]))
        if len(out) < k:
            for s, c, _b in self.ranked(unit, vaddr, size, k + len(out), mode="opcode"):
                if len(out) >= k:
                    break
                if (c["unit"], c["vaddr"]) not in seen:
                    out.append((s, "opcode", c))
                    seen.add((c["unit"], c["vaddr"]))
        return out


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


def neighbour_draft(row, sim, nb, body, mode="masked"):
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
    hdr = (NEIGHBOUR_HDR + " (tools/loop/sweep.py): %s 0x%08X %s, %s similarity %.3f */"
           % (nb["unit"], nb["vaddr"], nb["src"], mode, sim))
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


def oracle(u, vaddr, cpath, size, rodata, defsyms=(), extra=(), diffs=None, keep=None):
    """(clean?, defsyms, facts-summary, facts) with drive.py try's auto --defsym rerun.
    diffs: a list that gets the oracle's diff rows (drive.diff_rows) of a same-size MISMATCH.
    keep: the oracle's work dir (fn.i for m2cfix.repair()), else <draft>.work; removed here
    unless keep was given (match_fn keeps it on a miss)."""
    own = keep is None
    keep = keep or cpath[:-2] + ".work"
    try:
        return _oracle(u, vaddr, cpath, size, rodata, defsyms, extra, diffs, keep)
    finally:
        if own:
            shutil.rmtree(keep, ignore_errors=True)


def _oracle(u, vaddr, cpath, size, rodata, defsyms, extra, diffs, keep):
    defsyms = list(defsyms)
    argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep=keep) + list(extra)
    rc, facts, _text, err = drive.run_oracle(argv)
    if facts and facts.get("unresolved_symbols"):
        auto = []
        for s in facts["unresolved_symbols"]:
            m = drive.AUTO_SYM.match(s)
            if m:
                auto.append("%s=0x%s" % (s, m.group(2).upper()))
        if auto:
            defsyms = defsyms + auto
            argv = drive.oracle_argv(u, vaddr, cpath, size, rodata, defsyms, keep=keep) + list(extra)
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
            clean, defsyms, summ, facts = oracle(u, vaddr, cpath, size, rodata, keep=keep)
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


def note_try(res, variant, clean, facts, size, text):
    """Per-variant stats and the closest compiling draft so far (res["best"])."""
    pos, al = closeness(facts, size)
    res["stats"].append((variant, "match" if clean else ("compiled" if al is not None else "error"),
                         pos, al))
    if al is not None and (res["best"] is None or al > res["best"]["aligned"]):
        res["best"] = {"variant": variant, "aligned": al, "positional": pos, "text": text}


def work(row, workdir, variants, try_all=False, neighbours=None, nb_min=NEIGHBOUR_MIN, nb_tries=1, hb=None,
         nb_op_min=NEIGHBOUR_OP_MIN):
    """Try the variants in order on one body -> dict(result..., c_text, defsyms, variant).
    try_all: keep trying after the first clean one (to measure variants; the first wins).
    neighbours: a Neighbours index; the body's best neighbour goes to res["neighbour"]
    whether or not the `neighbour` variant is tried (it is only for similarity >= nb_min).
    `neighbour-op` is the backup: only when the best masked neighbour is under nb_min, the
    opcode-only neighbours at or above nb_op_min are drafted the same way.
    res["best"] is the compiling draft with the highest aligned closeness, res["stats"]
    one (variant, match|compiled|error|m2c_fail, positional, aligned) per try.
    hb: a heartbeat.Heartbeat told the variant being tried and each oracle run."""
    unit, vaddr = row["unit"], int(row["vaddr"], 16)
    hb = hb or heartbeat.Null()
    heartbeat.wait_if_paused(workdir, hb)
    hb.begin(unit, row["vaddr"], variants[0] if variants else "-", size=int(row["size"]))
    size = int(row["size"])
    u = units.resolve(unit)
    rodata = None
    if int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
        rodata = row["jt_addrs"].split(",")[0].strip()
    res = {"row": row, "tries": [], "match": None, "neighbour": [], "best": None, "stats": []}
    m2c_cache = {}
    if neighbours is not None:
        res["neighbour"] = neighbours.ranked(unit, vaddr, size, max(1, nb_tries))
    for variant in variants:
        if res["match"] and not try_all:
            break
        hb.update(unit, row["vaddr"], phase=variant)
        if variant in (NEIGHBOUR, NEIGHBOUR_OP):
            if variant == NEIGHBOUR:
                ranked, floor, mode = res["neighbour"], nb_min, "masked"
            elif res["neighbour"] and res["neighbour"][0][0] >= nb_min:
                continue                            # a close masked neighbour exists: no backup needed
            else:
                ranked = neighbours.ranked(unit, vaddr, size, max(1, nb_tries), mode="opcode") \
                    if neighbours is not None else []
                floor, mode = nb_op_min, "opcode"
            for k, (sim, nb, body) in enumerate(ranked):
                if sim < floor:
                    break
                name = variant if k == 0 else "%s%d" % (variant, k + 1)
                try:
                    text, defsyms, extra, nrod, _notes = neighbour_draft(row, sim, nb, body, mode)
                except (OSError, ValueError) as e:
                    res["tries"].append((name, "DRAFT_FAIL:%s" % e))
                    continue
                cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, name))
                with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write(text)
                rod = rodata or nrod
                diffs = []
                clean, defsyms, summ, facts = oracle(u, vaddr, cpath, size, rod, defsyms, extra, diffs)
                hb.add(compiles=1)
                res["tries"].append((name, summ))
                note_try(res, name, clean, facts, size, text)
                fixed = None if clean else imm_fixup(text, diffs)
                if fixed:                           # constants only: one more try with retail's
                    name += "+imm"
                    text = fixed
                    cpath = os.path.join(workdir, "%s_%08X_%s.c" % (unit, vaddr, name))
                    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
                        fh.write(text)
                    clean, defsyms, summ, facts = oracle(u, vaddr, cpath, size, rod, defsyms, extra)
                    hb.add(compiles=1)
                    res["tries"].append((name, summ))
                    note_try(res, name, clean, facts, size, text)
                if clean and not res["match"]:
                    res["match"] = {"variant": name, "text": text, "defsyms": defsyms, "rodata": rod,
                                    "extra": extra,
                                    "from": "%s draft of %s 0x%08X (%s, %s similarity %.3f)"
                                    % (name, nb["unit"], nb["vaddr"], nb["src"], mode, sim),
                                    "why": "%s of %s 0x%08X, %s similarity %.3f"
                                    % (name, nb["unit"], nb["vaddr"], mode, sim)}
                if clean:
                    break
            continue
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
        hb.add(compiles=1 + nfix)
        res["tries"].append((variant, summ + ("+r%d" % nfix if nfix else "")))
        note_try(res, variant, clean, facts, size, text)
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


TRY_WORDS = re.compile(r"MISMATCH@0x[0-9a-f]+/(\d+)")


def best_words(tries):
    """Fewest differing words over the MISMATCH tries, or None."""
    w = [int(m.group(1)) for _v, s in tries for m in [TRY_WORDS.search(s)] if m]
    return min(w) if w else None


REPORT_COLS = ["unit", "vaddr", "size", "group", "n_members", "status", "closeness", "positional",
               "best_variant", "tries", "neighbour"]


def print_variant_table(results, variants):
    """Per variant: drafts produced, compiled (cc1 ok), clean matches, mean aligned
    closeness of its compiling drafts, and bodies where it was the closest."""
    print("%-16s %6s %8s %8s %6s %7s %5s" % ("variant", "tried", "drafted", "compiled", "match",
                                            "mean~", "best"))
    for v in variants:
        st = [s for x in results for s in x.get("stats", []) if s[0].split("+")[0] == v]
        if not st:
            continue
        comp = [s for s in st if s[1] in ("compiled", "match")]
        best = sum(1 for x in results if x.get("best") and x["best"]["variant"].split("+")[0] == v)
        mean = sum(s[3] for s in comp) / len(comp) if comp else 0.0
        print("%-16s %6d %8d %8d %6d %7.3f %5d" % (
            v, len(st), sum(1 for s in st if s[1] != "m2c_fail"), len(comp),
            sum(1 for s in st if s[1] == "match"), mean, best))


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
                    % ", ".join(list(VARIANTS) + [NEIGHBOUR, NEIGHBOUR_OP]))
    ap.add_argument("--neighbour-min", type=float, default=NEIGHBOUR_MIN,
                    help="neighbour variant: lowest masked similarity tried (default %(default)s)")
    ap.add_argument("--neighbour-op-min", type=float, default=NEIGHBOUR_OP_MIN,
                    help="neighbour-op variant: lowest opcode-only similarity tried (default %(default)s)")
    ap.add_argument("--neighbour-tries", type=int, default=1,
                    help="neighbour variant: try the best N neighbours (default %(default)s)")
    ap.add_argument("--try-all", action="store_true",
                    help="try every variant even after a match (measures them; the first match wins)")
    ap.add_argument("--best-dir", help="write each missed body's closest compiling draft here "
                    "(<unit>_<ADDR>.c, header names the variant and closeness)")
    ap.add_argument("--dry-run", action="store_true", help="list the bodies, run nothing")
    ap.add_argument("--no-replay", action="store_true", help="do not run twins.py replay afterwards")
    ap.add_argument("--no-ledger", action="store_true")
    args = ap.parse_args(argv)

    variants = [v.strip() for v in args.variants.split(",") if v.strip()]
    bad = [v for v in variants if v not in VARIANTS and v not in (NEIGHBOUR, NEIGHBOUR_OP)]
    if bad:
        ap.error("unknown variant(s) %s; known: %s"
                 % (", ".join(bad), ", ".join(list(VARIANTS) + [NEIGHBOUR, NEIGHBOUR_OP])))
    nb_variants = [v for v in variants if v in (NEIGHBOUR, NEIGHBOUR_OP)]
    use_nb = bool(nb_variants)
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
        return nb_variants
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
    hb = heartbeat.Heartbeat(workdir, "sweep", len(rows), label=",".join(variants))
    with cf.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(work, r, workdir, row_variants(r), args.try_all, neighbours,
                          args.neighbour_min, args.neighbour_tries, hb, args.neighbour_op_min): r
                for r in rows}
        for i, f in enumerate(cf.as_completed(futs), 1):
            r = futs[f]
            try:
                res = f.result()
            except Exception as e:                  # noqa: BLE001 - one bad body must not stop the sweep
                res = {"row": r, "tries": [("-", "CRASH:%s" % e)], "match": None, "neighbour": [],
                       "best": None, "stats": []}
            res["status"] = commit_match(res, args) if res["match"] else "no match"
            results.append(res)
            b = res.get("best")
            hb.end(r["unit"], r["vaddr"], res["status"], words=best_words(res["tries"]),
                   variant=b["variant"] if b else None, closeness=round(b["aligned"], 3) if b else None,
                   tries=len(res["tries"]))
            if not res["tries"]:
                continue                            # nothing tried (no neighbour close enough)
            print("[%3d/%d] %-7s %s %4s B  %-8s %s" % (
                i, len(rows), r["unit"], r["vaddr"], r["size"],
                "MATCH" if res["status"] == "MATCH" else "-", " | ".join("%s %s" % t for t in res["tries"])))
            sys.stdout.flush()
    hb.close()
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
                                " | ".join("%s %s" % t for t in x["tries"]) or "-",
                                fmt_neighbour(x.get("neighbour"))]) + "\n")
    if args.best_dir:
        os.makedirs(args.best_dir, exist_ok=True)
        for x in results:
            r, b = x["row"], x.get("best")
            if b and x["status"] != "MATCH":
                p = os.path.join(args.best_dir, "%s_%s.c" % (r["unit"], r["vaddr"][2:].upper()))
                with open(p, "w", encoding="utf-8", newline="\n") as fh:
                    fh.write("/* %s %s  size %s  sweep.py best draft: %s, closeness %.3f aligned"
                             " / %.3f positional */\n%s" % (r["unit"], r["vaddr"], r["size"], b["variant"],
                                                            b["aligned"], b["positional"], b["text"]))
    print_variant_table(results, variants)
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
