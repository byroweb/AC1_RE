#!/usr/bin/env python3
"""regtie.py - register-tie tier: read GCC 2.7.2's register allocation, then steer it.

    regtie.py diagnose UNIT VADDR CFILE              which variable is in the wrong register, and why
    regtie.py run --cfile UNIT:CFILE ... [--evals 300] [--beam 6] [--no-ledger] [--dry-run]

Most near misses the permuter leaves are register ties: the C is right, but two values
with nearly equal allocation priority got each other's registers, and every word after
that shifts.  Random rewrites rarely flip one specific tie in a 2 KB function.  This tool
reads the compiler's own decisions instead of guessing:

diagnose  compiles the C exactly like the oracle, plus cc1 -dl -dg (the dumps change no
          code), and reads
            - retail vs ours: which of our registers retail has as which (aligned
              instructions that differ only in register names);
            - fn.i.lreg: every pseudo's weighted use count and live length ("Register N
              used X times across Y insns"), and fn.i.greg: global-alloc's allocation
              order;
            - pseudo -> hard register: the same insn (by uid) in both dumps;
            - variable -> hard register: the COFF debug info in fn.s (-gcoff is a pinned flag).
          Then it prints, for each register that retail spells differently, the variable
          sitting in it, the variable retail gave it to, and both priorities:
          priority = floor_log2(refs) * refs / live_length (global.c, refs weighted by
          loop depth).
run       a directed search per body: from the C, beam search over semantics-neutral
          rewrites (levers.py swap / commute / ptype / const / index / bytes) restricted
          to statements that name a variable of the tie, scored by
            (differing words, how far the tied variables' priorities are from flipping)
          so a rewrite that moves the tie the right way is kept even before the bytes
          change.  A score-0 text is written to re/match and judged by `drive.py try` (the
          oracle); only a clean MATCH gets its manifest + ledger rows.

One compile per evaluation, in-process like permute.py's lever stage: run it one job at
a time (it is single-threaded and small; it does not need the memory gate).
"""
import argparse
import collections
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
import levers  # noqa: E402

REGS = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"]
REGNUM = dict((n, i) for i, n in enumerate(REGS))
REGNUM["fp"] = 30
REG_TOKEN = re.compile(r"^(?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)$")
REG_LINE = re.compile(r"^Register (\d+) used (\d+) times across (\d+) insns( in block \d+)?", re.M)
ORDER = re.compile(r"^;; \d+ regs to allocate:([^\n]*)", re.M)
INSN = re.compile(r"^\((?:insn|jump_insn|call_insn) (\d+) ", re.M)
RTX_REG = re.compile(r"\(reg(/[a-z/]+)?:\w+ (\d+)(?: \$?\w+)?\)")
COFF_REG = re.compile(r"^\s*\.def\s+([A-Za-z_]\w*);\s*\.val\s+(\d+);\s*\.scl\s+(4|17);", re.M)
SEARCH_LEVERS = "swap,commute,ptype,const,index,bytes"


# ---------------------------------------------------------------------------
# compile with the allocation dumps
# ---------------------------------------------------------------------------
def build(meta, syms, text, inc):
    """-> dict(bytes, unresolved, asm, lreg, greg) or None when it does not build."""
    tools = meta["tools"]
    work = tempfile.mkdtemp(prefix="regtie_")
    try:
        c = os.path.join(work, "fn.c")
        with open(c, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        permute.preprocess(tools, c, os.path.join(work, "fn.i"), [inc])
        permute.run(match_fn.cc1_command(tools["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS + ["-dl", "-dg", "-o", "fn.s"],
                    work)
        with open(os.path.join(work, "fn.s"), encoding="utf-8", errors="replace") as fh:
            asm = fh.read()
        fixed = asm if meta.get("no_rodata_align_fix") else match_fn.fix_rodata_align(asm)[0]
        with open(os.path.join(work, "fn.maspsx.s"), "w", newline="\n") as fh:
            fh.write(permute.maspsx_inproc(tools["maspsx"], fixed))
        permute.run([tools["as"]] + match_fn.AS_FLAGS + ["-o", "fn.o", "fn.maspsx.s"], work)
        blob, unresolved = permute.resolve_object(os.path.join(work, "fn.o"), None, meta, syms)

        def dump(ext):
            p = os.path.join(work, "fn.i." + ext)
            if not os.path.isfile(p):
                return ""
            with open(p, encoding="utf-8", errors="replace") as fh:
                return fh.read()
        return {"bytes": blob, "unresolved": unresolved, "asm": asm, "lreg": dump("lreg"), "greg": dump("greg")}
    except (RuntimeError, match_fn.ToolError, OSError, ValueError):
        return None
    finally:
        shutil.rmtree(work, ignore_errors=True)


# ---------------------------------------------------------------------------
# reading the dumps
# ---------------------------------------------------------------------------
def floor_log2(n):
    return n.bit_length() - 1 if n > 0 else -1


def pseudos(lreg):
    """{pseudo: dict(refs, live, local, prio)} from the .lreg 'Register N used ...' lines."""
    out = {}
    for m in REG_LINE.finditer(lreg):
        n, refs, live = int(m.group(1)), int(m.group(2)), int(m.group(3))
        prio = floor_log2(refs) * refs / live if live else 0.0
        out[n] = {"refs": refs, "live": live, "local": bool(m.group(4)), "prio": prio}
    return out


def alloc_order(greg):
    m = ORDER.search(greg)
    return [int(x) for x in re.findall(r"\b(\d+)\b(?!\))", m.group(1))] if m else []


def insns(dump):
    """{uid: rtl text} for every insn of a dump."""
    marks = [(int(m.group(1)), m.start()) for m in INSN.finditer(dump)]
    out = {}
    for k, (uid, start) in enumerate(marks):
        end = marks[k + 1][1] if k + 1 < len(marks) else len(dump)
        out[uid] = dump[start:end]
    return out


def pseudo_regs(lreg, greg, first_pseudo):
    """{pseudo: (hard reg, user variable?)}: the same insn in the .lreg (pseudos) and the
    .greg (after allocation) dump, register operands paired in order, majority vote."""
    a, b = insns(lreg), insns(greg)
    votes = collections.defaultdict(collections.Counter)
    user = set()
    for uid, ta in a.items():
        tb = b.get(uid)
        if tb is None:
            continue
        ra = [(int(m.group(2)), "/v" in (m.group(1) or "")) for m in RTX_REG.finditer(ta)]
        rb = [int(m.group(2)) for m in RTX_REG.finditer(tb)]
        if len(ra) != len(rb):
            continue
        for (p, isuser), h in zip(ra, rb):
            if p >= first_pseudo and h < 32:
                votes[p][h] += 1
                if isuser:
                    user.add(p)
    return {p: (c.most_common(1)[0][0], p in user) for p, c in votes.items()}


def coff_vars(asm, fn):
    """{variable name: hard reg} for the register locals/params of fn in the COFF info."""
    i = asm.find(".def\t%s;" % fn)
    if i < 0:
        i = asm.find(".def %s;" % fn)
    body = asm[i:] if i >= 0 else asm
    j = body.find(".def\t.ef;")
    body = body[:j] if j > 0 else body
    return {m.group(1): int(m.group(2)) for m in COFF_REG.finditer(body)}


def disasm_rows(meta, blob):
    return permute.disasm_list(meta, blob)


def split_ops(ins):
    op, _, rest = ins.partition(" ")
    toks = re.split(r"[,()\s]+", rest.strip())
    return op, [t for t in toks if t]


def reg_map(retail_rows, our_rows):
    """Counter {(our reg, retail reg)} over aligned instructions that differ only in
    register names (positional when the sizes agree, else difflib on the opcodes)."""
    pairs = []
    if len(retail_rows) == len(our_rows):
        pairs = list(zip(our_rows, retail_rows))
    else:
        sm = difflib.SequenceMatcher(a=[split_ops(x)[0] for x in our_rows],
                                     b=[split_ops(x)[0] for x in retail_rows], autojunk=False)
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag == "equal":
                pairs += list(zip(our_rows[i1:i2], retail_rows[j1:j2]))
    out = collections.Counter()
    for o, r in pairs:
        oo, ot = split_ops(o)
        ro, rt = split_ops(r)
        if oo != ro or len(ot) != len(rt):
            continue
        diff = [(x, y) for x, y in zip(ot, rt) if x != y]
        if diff and all(REG_TOKEN.match(x) and REG_TOKEN.match(y) for x, y in diff):
            for x, y in diff:
                out[(x, y)] += 1
    return out


class Picture:
    """Everything diagnose knows about one compile."""

    def __init__(self, meta, res, retail):
        self.res = res
        self.ps = pseudos(res["lreg"])
        first = min(self.ps) if self.ps else 64
        self.order = alloc_order(res["greg"])
        self.rank = dict((p, i) for i, p in enumerate(self.order))
        self.hard = pseudo_regs(res["lreg"], res["greg"], first)
        self.vars = coff_vars(res["asm"], meta["fn"])
        self.words = permute.word_score(res["bytes"], retail)
        self.map = reg_map(disasm_rows(meta, retail), disasm_rows(meta, res["bytes"])) if self.words else {}

    def var_pseudo(self):
        """{variable: pseudo}: a register variable's hard reg names the user pseudos in it;
        several variables in one hard reg are paired in declaration order."""
        by_reg = collections.defaultdict(list)
        for p, (h, isuser) in sorted(self.hard.items()):
            if isuser:
                by_reg[h].append(p)
        names = collections.defaultdict(list)
        for v, h in self.vars.items():
            names[h].append(v)
        out = {}
        for h, vs in names.items():
            for v, p in zip(vs, by_reg.get(h, [])):
                out[v] = p
        return out

    def ties(self):
        """[(our reg, retail reg, count, [vars in our reg], [vars in retail's reg])], most common first."""
        names = collections.defaultdict(list)
        for v, h in self.vars.items():
            names[REGS[h]].append(v)
        out = []
        for (o, r), n in self.map.most_common():
            out.append((o, r, n, names.get(o, []), names.get(r, [])))
        return out

    def info(self, var):
        p = self.var_pseudo().get(var)
        d = self.ps.get(p, {}) if p is not None else {}
        return p, d


def describe(pic):
    vp = pic.var_pseudo()
    lines = ["diff words %d; register names retail spells differently: %d pairs" % (pic.words, len(pic.map))]
    for o, r, n, ours, theirs in pic.ties()[:8]:
        lines.append("  ours %-3s -> retail %-3s  x%-3d  ours holds %s; retail's %s is ours for %s"
                     % (o, r, n, ", ".join(ours) or "(a temporary)", r, ", ".join(theirs) or "(a temporary)"))
    shown = set()
    for _o, _r, _n, ours, theirs in pic.ties()[:8]:
        for v in ours + theirs:
            if v in shown or v not in vp:
                continue
            shown.add(v)
            p = vp[v]
            d = pic.ps.get(p, {})
            lines.append("    %-10s pseudo %-4d %-3s refs %-4s live %-5s priority %.3f  alloc rank %s%s"
                         % (v, p, REGS[pic.hard[p][0]], d.get("refs"), d.get("live"), d.get("prio", 0),
                            pic.rank.get(p, "-"), "  (local-alloc: one block)" if d.get("local") else ""))
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# directed search
# ---------------------------------------------------------------------------
def targets(pic):
    """Variable pairs of the main ties: (var that should move, var holding the register it wants)."""
    out = []
    for _o, _r, _n, ours, theirs in pic.ties()[:4]:
        for a in ours:
            for b in theirs:
                if a != b and (a, b) not in out:
                    out.append((a, b))
    return out


def tie_gap(pic, pairs):
    """Sum over pairs of how far each pair is from swapping allocation order (0 = flipped)."""
    vp = pic.var_pseudo()
    gap = 0.0
    for a, b in pairs:
        pa, pb = vp.get(a), vp.get(b)
        if pa is None or pb is None:
            continue
        ra, rb = pic.rank.get(pa), pic.rank.get(pb)
        if ra is None or rb is None:
            continue
        # a sits in a register b should have, so a was allocated before b: want b first
        pra, prb = pic.ps.get(pa, {}).get("prio", 0), pic.ps.get(pb, {}).get("prio", 0)
        gap += max(0.0, pra - prb) / (max(pra, prb) or 1.0) if ra < rb else 0.0
    return gap


DECL_VAR = re.compile(r"^([ \t]*)((?:register |volatile )?)(s8|u8|s16|u16|s32|u32|int|short|char|long|unsigned int|"
                      r"unsigned short|unsigned char) (\w+)( = [^;]+)?;", re.M)
RETYPE = ["s32", "u32", "s16", "u16", "s8", "u8"]
CONTROL = re.compile(r"\s*(return|break|continue|goto|case|default)\b")


def lever_move(text, fn, names):
    """A statement that names a tied variable, moved 1-3 places up or down inside its own
    block (same indentation, nothing but statements in between): its live range changes."""
    sp = levers.fn_span(text, fn)
    if not sp or not names:
        return []
    pat = re.compile(r"\b(?:%s)\b" % "|".join(re.escape(n) for n in names))
    body = text[sp[0]:sp[1]]
    stmts = [m for m in levers.STMT.finditer(body) if not CONTROL.match(m.group(2))]
    out = []
    for i, a in enumerate(stmts):
        if not pat.search(a.group(2)):
            continue
        for d in (-3, -2, -1, 1, 2, 3):
            j = i + d
            if j < 0 or j >= len(stmts):
                continue
            lo, hi = (j, i) if d < 0 else (i, j)
            run = stmts[lo:hi + 1]
            if any(x.group(1) != a.group(1) for x in run):
                continue
            if any(body[x.end():y.start()].strip() for x, y in zip(run, run[1:])):
                continue
            order = [x.group(2) for x in run]
            moved = order.pop(i - lo)
            order.insert(j - lo, moved)
            seg = "\n".join(a.group(1) + t for t in order)
            nb = body[:run[0].start()] + seg + body[run[-1].end():]
            out.append(("move %s %+d" % (a.group(2).strip()[:28], d), text[:sp[0]] + nb + text[sp[1]:]))
    return out


def lever_retype(text, fn, names):
    """A tied variable's declared integer type, changed: its use count (extensions, compares) changes.
    Only bytes judge the result, so a match is exactly retail's code either way."""
    sp = levers.fn_span(text, fn)
    if not sp:
        return []
    body = text[sp[0]:sp[1]]
    out = []
    for m in DECL_VAR.finditer(body):
        if m.group(4) not in names:
            continue
        for t in RETYPE:
            if t == m.group(3):
                continue
            line = "%s%s%s %s%s;" % (m.group(1), m.group(2), t, m.group(4), m.group(5) or "")
            nb = body[:m.start()] + line + body[m.end():]
            out.append(("retype %s %s -> %s" % (m.group(4), m.group(3), t), text[:sp[0]] + nb + text[sp[1]:]))
    return out


def lever_split(text, fn, names, pairs=()):
    """A tied variable reused for a later, unrelated value is one allocno for both live
    ranges in this GCC.  From its n-th assignment to the end of the function, give it a
    fresh name (split) or the name of the variable it is tied with (reuse)."""
    sp = levers.fn_span(text, fn)
    if not sp:
        return []
    body = text[sp[0]:sp[1]]
    decls = {m.group(4): m for m in DECL_VAR.finditer(body)}
    partner = collections.defaultdict(set)
    for x, y in pairs:
        partner[x].add(y)
        partner[y].add(x)
    out = []
    for v in names:
        d = decls.get(v)
        if not d:
            continue
        assigns = [m for m in re.finditer(r"(?m)^[ \t]*%s\s*=[^=]" % re.escape(v), body) if m.start() > d.end()]
        for k, m in enumerate(assigns):
            if k:                                          # split: from a later assignment on
                tail = re.sub(r"\b%s\b" % re.escape(v), "%s_b" % v, body[m.start():])
                decl = "%s%s%s %s_b;" % (d.group(1), d.group(2), d.group(3), v)
                nb = body[:d.end()] + "\n" + decl + body[d.end():m.start()] + tail
                out.append(("split %s at %s" % (v, body[m.start():m.start() + 30].strip()),
                            text[:sp[0]] + nb + text[sp[1]:]))
            for w in sorted(partner[v]):                   # reuse: from any assignment on
                if w in decls and decls[w].group(3) == d.group(3):
                    tail = re.sub(r"\b%s\b" % re.escape(v), w, body[m.start():])
                    nb = body[:m.start()] + tail
                    out.append(("reuse %s for %s from %s" % (w, v, body[m.start():m.start() + 24].strip()),
                                text[:sp[0]] + nb + text[sp[1]:]))
    return out


def candidates(text, fn, names, pairs=()):
    out = lever_split(text, fn, names, pairs) + lever_move(text, fn, names) + lever_retype(text, fn, names)
    out += [(d, c) for d, c in levers.variants(text, fn, SEARCH_LEVERS) if mentions(text, c, names)]
    return out


def mentions(old, new, names):
    """Does the edited region (lines that differ) name one of the variables?"""
    if not names:
        return True
    pat = re.compile(r"\b(?:%s)\b" % "|".join(re.escape(n) for n in names))
    for line in difflib.unified_diff(old.splitlines(), new.splitlines(), n=0, lineterm=""):
        if line[:1] in "+-" and not line.startswith(("+++", "---")) and pat.search(line):
            return True
    return False


def search(meta, syms, text, inc, retail, evals=300, beam=6, log=print):
    """-> dict(best text, best words, evals, trail)."""
    res = build(meta, syms, text, inc)
    if res is None:
        return {"text": None, "words": None, "evals": 1, "trail": [], "note": "draft does not build"}
    pic = Picture(meta, res, retail)
    pairs = targets(pic)
    names = sorted({v for p in pairs for v in p})
    log(describe(pic))
    log("targets: %s" % (", ".join("%s<->%s" % p for p in pairs) or "(none: unrestricted levers)"))
    start = (pic.words, tie_gap(pic, pairs))
    frontier = [(start, text, [])]
    best = frontier[0]
    seen = {res["bytes"]}
    n = 1
    while frontier and n < evals and best[0][0] > 0:
        nxt = []
        for score, t, trail in frontier:
            for desc, cand in candidates(t, meta["fn"], names, pairs):
                if n >= evals:
                    break
                r = build(meta, syms, cand, inc)
                n += 1
                if r is None or r["bytes"] in seen:
                    continue
                seen.add(r["bytes"])
                p = Picture(meta, r, retail)
                s = (p.words, tie_gap(p, pairs))
                nxt.append((s, cand, trail + [desc]))
                if s < best[0]:
                    best = (s, cand, trail + [desc])
                    log("  %4d evals: %d words, tie gap %.3f  <- %s" % (n, s[0], s[1], desc))
                    if s[0] == 0:
                        break
            if best[0][0] == 0 or n >= evals:
                break
        nxt.sort(key=lambda x: x[0])
        # the best `beam` children go on, worse ones too: a tie is often flipped two edits away
        frontier = nxt[:beam]
    return {"text": best[1], "words": best[0][0], "gap": best[0][1], "evals": n, "trail": best[2],
            "start": start[0], "note": ""}


# ---------------------------------------------------------------------------
# per-function oracle options from a --cfile spec (UNIT:PATH@rodata=VA@noalign), as promote.py
# spells them: a jump table's .rodata start, and --no-rodata-align-fix for 8-aligned tables
OPTS = {}


def load(unit, vaddr, cfile, work):
    d = os.path.join(work, "%s_%08X" % (unit, vaddr))
    o = OPTS.get((unit, vaddr), {})
    meta = permute.setup_dir(unit, vaddr, cfile, d, rodata=o.get("rodata"), tools=permute.tools_once())
    if o.get("noalign"):
        meta["no_rodata_align_fix"] = True
    syms = permute.symtab(meta)
    retail = permute.retail_bytes(unit, vaddr, meta["size"])
    with open(cfile, encoding="utf-8") as fh:
        text = fh.read()
    return meta, syms, retail, text


def parse_cfile(spec):
    """UNIT:PATH[@rodata=VA][@noalign] -> (unit, vaddr, path); the options go to OPTS."""
    spec = spec.strip()                                  # a list file written on Windows ends lines in \r
    spec, *opts = spec.split("@")
    unit, path = spec.split(":", 1) if re.match(r"^(exe|ovl20[1-4]|fdat\d{3}):", spec) else (None, spec)
    m = re.search(r"func_([0-9A-Fa-f]{8})", os.path.basename(path))
    if not unit or not m:
        raise SystemExit("regtie.py: --cfile wants UNIT:PATH with func_<ADDR> in the name: %s" % spec)
    vaddr = int(m.group(1), 16)
    o = {}
    for x in opts:
        if x.startswith("rodata="):
            o["rodata"] = "0x%08X" % int(x.split("=", 1)[1], 16)
        elif x == "noalign":
            o["noalign"] = True
    if o:
        OPTS[(unit, vaddr)] = o
    return unit, vaddr, path


def commit(unit, vaddr, text, trail, no_ledger):
    """Write to re/match and let drive.py try judge it (manifest + ledger on a clean MATCH)."""
    import drive
    vs = "0x%08X" % vaddr
    cpath = drive.c_path(drive.RE_MATCH, unit, vs)
    if os.path.exists(cpath):
        return "skipped: %s exists" % drive.rel(cpath)
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("/* %s %s  register-tie tier (tools/loop/regtie.py): %s */\n%s"
                 % (unit, vs, " + ".join(trail)[:200] or "as drafted", text))
    env = dict(os.environ)
    if no_ledger:
        env["AC1_LEDGER"] = os.path.join(tempfile.gettempdir(), "regtie_scratch_ledger.tsv")
    o = OPTS.get((unit, vaddr), {})
    extra = (["--rodata", o["rodata"]] if o.get("rodata") else []) + (["--no-rodata-align-fix"] if o.get("noalign") else [])
    r = subprocess.run([sys.executable, os.path.join(HERE, "drive.py"), "try", unit, vs] + extra, cwd=REPO, env=env,
                       capture_output=True, text=True, errors="replace")
    if r.returncode != 0:
        os.unlink(cpath)
        last = [l for l in r.stdout.splitlines() if l.startswith("result:")]
        return "re-verify failed (%s)" % (last[0] if last else r.stdout[-200:])
    return "MATCH"


def cmd_diagnose(args):
    unit, vaddr, path = (args.unit, int(args.vaddr, 16), args.cfile)
    work = tempfile.mkdtemp(prefix="regtie_dx_")
    try:
        meta, syms, retail, text = load(unit, vaddr, path, work)
        res = build(meta, syms, text, os.path.dirname(os.path.abspath(path)))
        if res is None:
            print("does not build")
            return 2
        print(describe(Picture(meta, res, retail)))
    finally:
        shutil.rmtree(work, ignore_errors=True)
    return 0


def cmd_run(args):
    work = os.path.abspath(args.work) if args.work else tempfile.mkdtemp(prefix="regtie_run_")
    os.makedirs(work, exist_ok=True)
    report = os.path.join(work, "regtie_report.tsv")
    rows = []
    for k, spec in enumerate(args.cfile, 1):
        unit, vaddr, path = parse_cfile(spec)
        print("[%d/%d] %s 0x%08X  %s" % (k, len(args.cfile), unit, vaddr, path))
        sys.stdout.flush()
        try:
            meta, syms, retail, text = load(unit, vaddr, path, work)
            out = search(meta, syms, text, os.path.dirname(os.path.abspath(path)), retail, args.evals, args.beam,
                         log=lambda m: (print("  " + m.replace("\n", "\n  ")), sys.stdout.flush()))
        except Exception as e:  # noqa: BLE001 - one bad body must not stop the run
            out = {"text": None, "words": None, "evals": 0, "trail": [], "note": "%s: %s" % (type(e).__name__, e)}
        status = "no match"
        if out.get("words") == 0 and out["text"] and not args.dry_run:
            status = commit(unit, vaddr, out["text"], out["trail"], args.no_ledger)
        elif out.get("words") == 0:
            status = "zero (dry run)"
        if out.get("text") and out.get("words") and out["words"] < (out.get("start") or 1 << 30):
            with open(os.path.join(work, "%s_%08X_best.c" % (unit, vaddr)), "w", encoding="utf-8",
                      newline="\n") as fh:
                fh.write(out["text"])
        print("  -> %s: words %s -> %s in %d evals %s" % (status, out.get("start"), out.get("words"), out["evals"],
                                                          out.get("note") or " + ".join(out["trail"])[:120]))
        sys.stdout.flush()
        rows.append([unit, "0x%08X" % vaddr, str(out.get("start")), str(out.get("words")), str(out["evals"]), status,
                     " + ".join(out["trail"])[:300], out.get("note", "")])
    with open(report, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("unit\tvaddr\tstart_words\tbest_words\tevals\tstatus\ttrail\tnote\n")
        for r in rows:
            fh.write("\t".join(r) + "\n")
    hits = sum(1 for r in rows if r[5] == "MATCH")
    print("regtie: %d/%d matched; report %s" % (hits, len(rows), report))
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("diagnose")
    p.add_argument("unit")
    p.add_argument("vaddr")
    p.add_argument("cfile")
    p = sub.add_parser("run")
    p.add_argument("--cfile", action="append", required=True, metavar="UNIT:C")
    p.add_argument("--evals", type=int, default=300, help="compiles per body (default %(default)s)")
    p.add_argument("--beam", type=int, default=6)
    p.add_argument("--work")
    p.add_argument("--no-ledger", action="store_true")
    p.add_argument("--dry-run", action="store_true", help="never write re/match")
    args = ap.parse_args(argv)
    return {"diagnose": cmd_diagnose, "run": cmd_run}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
