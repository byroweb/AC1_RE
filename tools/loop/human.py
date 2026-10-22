"""human.py - the viewer's human controls (tools/loop/viewer.py).

For someone who knows the game but not MIPS or GCC 2.7.2:

  classify()   the oracle's retail | ours rows -> plain-language symptoms ("same
               instructions, different registers", "a branch flipped", ...), each with
               the moves worth a try
  explain()    one paragraph on what the current diff means
  Tries        one click = one background job on the body's current C: a lever family
               from tools/loop/levers.py (every site, scored with the oracle's own compile,
               like tools/match/variants.py), a swap of two lines the person picked, or
               decomp-permuter for N minutes (permute.py).  A better (or equal-score,
               different-code) text becomes a numbered version in <work>/human/<unit>_<ADDR>/
               that the viewer shows as a source; undo shelves the newest one.  Score 0
               only counts after match_fn.py says MATCH (sweep.oracle), and only
               record_match() writes re/match, the manifest and the ledger.
  line_map()   asm offset -> C line, from cc1's .loc lines (objdump -l on the oracle's
               fn.o gives fn.i lines; the function bodies of fn.i and the C are aligned
               line by line)
  log_action() re/match/actions.tsv: every try, undo, steer and record, with the score
               before and after, so later passes can see which human moves worked.
"""
import concurrent.futures as cf
import datetime
import difflib
import hashlib
import itertools
import os
import re
import shutil
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))

ACTIONS = os.path.join(REPO, "re", "match", "actions.tsv")
ACTION_COLS = ["timestamp", "unit", "vaddr", "action", "detail", "before", "after", "result"]
MAX_PERMUTER_S = 30 * 60

# ---------------------------------------------------------------------------
# symptoms and moves, in plain words
# ---------------------------------------------------------------------------
SYMPTOMS = {
    "registers": ("same instructions, different registers",
                  "The steps are right but values sit in different registers. GCC hands out "
                  "registers in the order values are first computed, so the order of two "
                  "statements (or which side of a + comes first) usually decides it."),
    "order": ("instructions in a different order",
              "The same instructions are all there, just shuffled. GCC 2.7.2 keeps the order "
              "the source wrote things in, so two statements are probably the other way round."),
    "extra": ("an extra or missing instruction",
              "One side has an instruction the other lacks, which shifts everything after it. "
              "Often a call argument the decompiler dropped, a value used right after it was "
              "loaded (a wait slot), or a statement too many or too few."),
    "constant": ("a different constant or offset",
                 "Same instruction, different number: a struct field offset, an array step, or "
                 "a constant written another way (-1 versus 0xFF, a negative versus unsigned)."),
    "type": ("a load or store of the wrong size or sign",
             "Retail reads the value as signed and ours as unsigned (or byte versus short): "
             "the variable or field has the wrong type."),
    "branch": ("a branch flipped",
               "A test goes the other way (equal versus not equal, less versus greater-or-"
               "equal): the if and else parts, or the comparison, are the other way round."),
    "call": ("a different function called",
             "A call goes somewhere else: a wrong function name or address."),
    "other": ("other differences",
              "Differences that do not fit one pattern; the first one is usually the cause "
              "and the rest follow from it."),
}
MOVES = {
    "swap": ("Swap neighbouring statements", "tries each pair of neighbouring statements the other way round"),
    "commute": ("Flip the sides of + & | ^ *", "writes b + a instead of a + b, one place at a time"),
    "args": ("Pass a dropped argument through", "adds a call argument the decompiler left out"),
    "bytes": ("Count pointer steps in bytes", "treats `p += 8` as 8 bytes rather than 8 elements"),
    "ptype": ("Change what a pointer points at", "signed / unsigned, byte / short / word, one pointer at a time"),
    "etype": ("Give a global a byte or short type", "for globals used as a base address"),
    "const": ("Write a constant the other way", "-1 versus 0xFF / 0xFFFF, -16 versus 0xF0"),
    "index": ("Name a raw address as an array", "turns a fixed address plus an index into a named array"),
    "permuter": ("Let the permuter chew on it", "random small rewrites, kept when they score better"),
}
SUGGEST = {"registers": ["swap", "commute", "permuter"], "order": ["swap", "permuter"],
           "extra": ["args", "bytes", "permuter"], "constant": ["const", "index", "bytes"],
           "type": ["ptype", "etype", "const"], "branch": ["swap", "permuter"], "call": ["args"],
           "other": ["swap", "permuter"]}


def _d():
    import drive
    return drive


def row_kind(a, b, pool_a, pool_b):
    """Symptom of one differing row (retail a, ours b); pool_*: the other side's differing
    instructions (for `order`)."""
    d = _d()
    ma, mb = d.mnem(a), d.mnem(b)
    if a in ("--", "") or b in ("--", "") or (ma == "nop") != (mb == "nop"):
        return "extra"
    if d.INVERSE.get(ma) == mb:
        return "branch"
    if (ma, mb) in d.SIGN_PAIRS:
        return "type"
    if ma == mb == "jal" or ma == mb == "j":
        return "call" if ma == "jal" else "other"
    if ma == mb and d.regs_norm(a) == d.regs_norm(b):
        return "registers"
    if ma == mb and d.imm_norm(a) == d.imm_norm(b):
        return "constant"
    if a in pool_b and b in pool_a:
        return "order"
    if d.regs_norm(a) in {d.regs_norm(x) for x in pool_b}:
        return "order"
    return "other"


def classify(rows, size=None, ours_size=None):
    """[{kind, label, why, count, first, moves:[{id, label, why}]}], biggest first."""
    diff = [r for r in rows if r[0] == "!"]
    pool_a = [r[2] for r in diff]
    pool_b = [r[3] for r in diff]
    count, first = {}, {}
    for _m, addr, a, b in diff:
        k = row_kind(a, b, pool_a, pool_b)
        count[k] = count.get(k, 0) + 1
        first.setdefault(k, addr)
    if size and ours_size and ours_size != size:
        count["extra"] = count.get("extra", 0) + abs(ours_size - size) // 4
        first.setdefault("extra", None)
    out = []
    for k in sorted(count, key=lambda k: (-count[k], list(SYMPTOMS).index(k))):
        out.append({"kind": k, "label": SYMPTOMS[k][0], "why": SYMPTOMS[k][1], "count": count[k],
                    "first": first[k],
                    "moves": [{"id": m, "label": MOVES[m][0], "why": MOVES[m][1]} for m in SUGGEST[k]]})
    return out


def explain(symptoms, result=None, diff_words=None, size=None, ours_size=None):
    """One plain-English paragraph about the current diff."""
    if result == "MATCH":
        return "Our compiled code is byte for byte the retail code: this function is matched."
    if result == "ERROR":
        return ("Our C does not compile yet, so there is nothing to compare: the right column is "
                "empty until the build error above is fixed (or another source is picked).")
    if not symptoms:
        return "No differing instructions in this comparison."
    words = (size or 0) // 4
    top = symptoms[0]
    s = ("Of the %d instructions in the retail function, %d differ in ours. " % (words, diff_words)
         if words and diff_words is not None else "")
    if ours_size and size and ours_size != size:
        s += "Ours is %d instruction(s) %s, so everything after the first gap is shifted. " % (
            abs(ours_size - size) // 4, "longer" if ours_size > size else "shorter")
    s += "Mostly %s (%d): %s" % (top["label"], top["count"], top["why"])
    rest = [x for x in symptoms[1:] if x["count"]]
    if rest:
        s += " Also: " + ", ".join("%s (%d)" % (x["label"], x["count"]) for x in rest[:3]) + "."
    s += " Worth a try: " + " or ".join(m["label"].lower() for m in top["moves"]) + "."
    return s


# ---------------------------------------------------------------------------
# asm offset -> C line
# ---------------------------------------------------------------------------
OBJ_LINE = re.compile(r"fn\.i:(\d+)(?:\s+\(discriminator \d+\))?\s*$")
OBJ_INSN = re.compile(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s")


def objdump_lines(text):
    """objdump -dl output -> {byte offset: fn.i line}."""
    out, cur = {}, None
    for ln in text.splitlines():
        m = OBJ_LINE.search(ln)
        if m:
            cur = int(m.group(1))
            continue
        m = OBJ_INSN.match(ln)
        if m and cur is not None:
            out[int(m.group(1), 16)] = cur
    return out


def body_lines(text, fn):
    """(first line number, [lines]) of fn's definition, or None."""
    import permute
    sp = permute.find_function(text, fn)
    if not sp:
        return None
    first = text.count("\n", 0, sp[0]) + 1
    body = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text[sp[0]:sp[1]], flags=re.S)
    return first, [re.sub(r"//.*", "", x) for x in body.split("\n")]    # comments out: cpp drops them


def _norm(s):
    return re.sub(r"\s+", "", s)


def map_lines(fni_text, c_text, fn):
    """{fn.i line: C line} by aligning the two function bodies (cpp drops comments and
    blank lines, expands macros: equal lines pair up, replaced runs pair in order)."""
    a, b = body_lines(fni_text, fn), body_lines(c_text, fn)
    if not a or not b:
        return {}
    (fa, la), (fb, lb) = a, b
    out = {}
    sm = difflib.SequenceMatcher(None, [_norm(x) for x in la], [_norm(x) for x in lb], autojunk=False)
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                out[fa + i1 + k] = fb + j1 + k
        elif tag == "replace":          # pair in order, skipping the C side's blank lines
            js = [j for j in range(j1, j2) if lb[j].strip()] or list(range(j1, j2))
            for k in range(i2 - i1):
                out[fa + i1 + k] = fb + js[min(k, len(js) - 1)]
        elif tag == "delete" and j1 > 0:
            for k in range(i2 - i1):
                out[fa + i1 + k] = fb + j1 - 1
    return out


def line_map(objdump_text, fni_text, c_text, fn):
    """{byte offset: C line (1-based)} for our compiled function."""
    il = objdump_lines(objdump_text)
    m = map_lines(fni_text, c_text, fn)
    return {off: m[l] for off, l in il.items() if l in m}


# ---------------------------------------------------------------------------
# versions (the person's tries) and the action log
# ---------------------------------------------------------------------------
def now():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def log_action(unit, vaddr, action, detail="", before="", after="", result="", path=None):
    path = path or ACTIONS
    new = not os.path.isfile(path) or os.path.getsize(path) == 0
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    row = [now(), unit, vaddr, action, detail, before, after, result]
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(ACTION_COLS) + "\n")
        fh.write("\t".join(" ".join(str("" if x is None else x).split()) for x in row) + "\n")


class Versions:
    """<root>/<unit>_<ADDR>/vNNN.c + versions.tsv (n, label, score, result, from, time)."""
    COLS = ["n", "label", "score", "result", "from", "t"]

    def __init__(self, root, unit, vaddr):
        self.dir = os.path.join(root, "%s_%s" % (unit, vaddr[2:]))
        self.index = os.path.join(self.dir, "versions.tsv")

    def list(self):
        out = []
        if not os.path.isfile(self.index):
            return out
        with open(self.index, encoding="utf-8") as fh:
            for line in fh:
                p = line.rstrip("\n").split("\t")
                if p[0] == "n" or len(p) < len(self.COLS):
                    continue
                r = dict(zip(self.COLS, p))
                r["path"] = os.path.join(self.dir, "v%03d.c" % int(r["n"]))
                if os.path.isfile(r["path"]):
                    r["n"] = int(r["n"])
                    r["t"] = float(r["t"])
                    r["score"] = int(r["score"]) if r["score"].lstrip("-").isdigit() else None
                    out.append(r)
        return out

    def add(self, text, label, score, result, src):
        os.makedirs(self.dir, exist_ok=True)
        n = max([0] + [int(f[1:4]) for f in os.listdir(self.dir) if re.match(r"^v\d{3}\.c", f)]) + 1
        with open(os.path.join(self.dir, "v%03d.c" % n), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        new = not os.path.isfile(self.index)
        with open(self.index, "a", encoding="utf-8", newline="\n") as fh:
            if new:
                fh.write("\t".join(self.COLS) + "\n")
            fh.write("\t".join(" ".join(str(x).split()) for x in (n, label, score, result, src, time.time())) + "\n")
        return n

    def undo(self):
        """Shelve the newest version (vNNN.c -> vNNN.c.undone). -> its row or None."""
        vs = self.list()
        if not vs:
            return None
        v = vs[-1]
        os.replace(v["path"], v["path"] + ".undone")
        return v


# ---------------------------------------------------------------------------
# tries: background jobs
# ---------------------------------------------------------------------------
def swap_lines(text, a, b):
    """text with (1-based) lines a and b exchanged; None when out of range or equal."""
    lines = text.split("\n")
    if a == b or not (1 <= a <= len(lines) and 1 <= b <= len(lines)):
        return None
    lines[a - 1], lines[b - 1] = lines[b - 1], lines[a - 1]
    return "\n".join(lines)


class Target:
    """The oracle's compile set-up for one body (variants.py's make_meta, in process)."""

    def __init__(self, unit, vaddr, size, text):
        import drive
        import permute
        self.unit, self.vaddr, self.size = unit, vaddr, size
        self.fn = "func_%s" % vaddr[2:]
        row = drive.load_rows().get((unit, vaddr))
        rodata = None
        if row and int(row.get("jump_tables") or 0) and row.get("jt_addrs"):
            rodata = row["jt_addrs"].split(",")[0].strip()
        self.meta = {"unit": unit, "vaddr": vaddr, "fn": self.fn, "size": size, "rodata": rodata,
                     "syms": permute.SYMS_TABLES if drive.AC1_INCLUDE.search(text) else [], "defsyms": [],
                     "tools": permute.tools_once()}
        self.syms = permute.symtab(self.meta)
        self.retail = permute.retail_bytes(unit, int(vaddr, 16), size)

    def score(self, text, inc_dirs=()):
        """-> (word score or None, code hash or None, error or None)."""
        import permute
        import variants
        ours, _unres, err = variants.build(self.meta, self.syms, text, list(inc_dirs))
        if ours is None:
            return None, None, err
        return permute.word_score(ours, self.retail), hashlib.sha1(ours).hexdigest(), None

    def oracle(self, path):
        """match_fn.py on a file: -> (clean, defsyms, summary)."""
        import sweep
        import units
        out = sweep.oracle(units.resolve(self.unit), int(self.vaddr, 16), path, self.size, self.meta["rodata"])
        return out[0], out[1], out[2]


class Tries:
    """Runs the person's moves two at a time; jobs and results stay in memory."""

    def __init__(self, root, actions=None, jobs=2, compile_jobs=8):
        self.root = root
        self.actions = actions
        self.pool = cf.ThreadPoolExecutor(max_workers=jobs)
        self.compile_jobs = compile_jobs
        self.jobs = {}
        self.lock = threading.Lock()
        self.ids = itertools.count(1)

    def versions(self, unit, vaddr):
        return Versions(self.root, unit, vaddr)

    def list(self, key=None):
        with self.lock:
            js = [dict(j) for j in self.jobs.values() if key is None or j["k"] == key]
        return sorted(js, key=lambda j: -j["id"])

    def submit(self, unit, vaddr, size, text, src, move, minutes=None, lines=None):
        if move not in MOVES and move != "swaplines":
            raise ValueError("unknown move %r" % move)
        if move == "swaplines" and (not lines or len(lines) != 2):
            raise ValueError("swaplines wants two line numbers")
        if move == "permuter":
            minutes = float(minutes or 5)
            if not 0 < minutes * 60 <= MAX_PERMUTER_S:
                raise ValueError("permuter time is 1 to %d minutes" % (MAX_PERMUTER_S // 60))
        jid = next(self.ids)
        label = (MOVES[move][0] if move in MOVES else "Swap lines %d and %d" % tuple(lines))
        if move == "permuter":
            label += " (%g min)" % minutes
        job = {"id": jid, "k": "%s %s" % (unit, vaddr), "move": move, "label": label, "status": "queued",
               "queued": time.time(), "src": src, "result": None}
        with self.lock:
            self.jobs[jid] = job
        self.pool.submit(self._run, job, unit, vaddr, size, text, move, minutes, lines)
        return dict(job)

    def _set(self, job, **kw):
        with self.lock:
            job.update(kw)

    def _run(self, job, unit, vaddr, size, text, move, minutes, lines):
        self._set(job, status="running", started=time.time())
        try:
            res = self._try(job, unit, vaddr, size, text, move, minutes, lines)
            self._set(job, status="done", result=res, ended=time.time())
        except Exception as e:      # noqa: BLE001 - shown on the page
            res = {"error": "%s: %s" % (type(e).__name__, e)}
            self._set(job, status="failed", result=res, ended=time.time())
        log_action(unit, vaddr, "try", job["label"] + " on " + job["src"], (res or {}).get("before"),
                   (res or {}).get("after"), (res or {}).get("outcome") or (res or {}).get("error"), self.actions)

    def _try(self, job, unit, vaddr, size, text, move, minutes, lines):
        tg = Target(unit, vaddr, size, text)
        before, base_code, err = tg.score(text)
        if before is None:
            return {"error": "the current C does not build: %s" % err, "outcome": "does not build"}
        cands = []
        if move == "swaplines":
            t = swap_lines(text, *lines)
            if t is None:
                return {"before": before, "outcome": "nothing to swap"}
            cands = [("swap lines %d and %d" % tuple(lines), t)]
        elif move == "permuter":
            return self._permuter(job, tg, text, before, minutes)
        else:
            import levers
            cands = levers.variants(text, tg.fn, move)
            if not cands:
                return {"before": before, "after": before, "tried": 0,
                        "outcome": "no place in this function where that move applies"}
        self._set(job, progress="%d variant(s) to compile" % len(cands))
        with cf.ThreadPoolExecutor(max_workers=self.compile_jobs) as ex:
            scored = list(ex.map(lambda c: (c[0], c[1]) + tg.score(c[1]), cands))
        built = [s for s in scored if s[2] is not None and s[3] != base_code]
        if not built:
            errs = [s[4] for s in scored if s[2] is None]
            why = ("it does not compile: %s" % errs[0] if len(errs) == len(scored)
                   else "the compiler made exactly the same code")
            return {"before": before, "after": before, "tried": len(cands), "outcome": "no change: " + why}
        label, best_text, after, _code, _e = min(built, key=lambda s: s[2])
        return self._keep(job, tg, best_text, label, before, after, len(cands))

    def _keep(self, job, tg, text, label, before, after, tried):
        res = {"before": before, "after": after, "delta": after - before, "tried": tried, "label": label}
        if after > before:
            res["outcome"] = "worse (best try %+d): nothing kept" % (after - before)
            return res
        vs = self.versions(tg.unit, tg.vaddr)
        result = "better" if after < before else "same score, different code"
        if after == 0:
            tmp = os.path.join(vs.dir, "check")
            os.makedirs(tmp, exist_ok=True)
            p = os.path.join(tmp, "%s.c" % tg.fn)
            with open(p, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(text)
            clean, defsyms, summ = tg.oracle(p)
            shutil.rmtree(tmp, ignore_errors=True)
            result = "MATCH" if clean else "score 0 but match_fn.py says %s" % summ
            res["match"] = clean
            res["defsyms"] = defsyms
        n = vs.add(text, "%s: %s" % (job["label"], label), after, result, job["src"])
        res.update(version=n, outcome=result)
        return res

    def _permuter(self, job, tg, text, before, minutes):
        import permute
        permute.permuter_dir()                  # SystemExit with the how-to when it is missing
        vs = self.versions(tg.unit, tg.vaddr)
        d = os.path.join(vs.dir, "perm%d" % job["id"])
        os.makedirs(d, exist_ok=True)
        src = os.path.join(d, "start.c")
        with open(src, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        permute.setup_dir(tg.unit, int(tg.vaddr, 16), src, d, size=tg.size, rodata=tg.meta["rodata"],
                          tools=tg.meta["tools"])

        def tick(iterations=None, best=None):
            if iterations is not None:
                self._set(job, progress="%d iterations" % iterations)
            if best is not None:
                self._set(job, perm_best=best)
        r = permute.run_permuter(d, minutes * 60, os.path.join(d, "permuter.log"), tick)
        if r["failed"]:
            return {"before": before, "error": r["failed"], "outcome": "permuter failed"}
        outs = sorted((int(x.split("-")[1]), x) for x in os.listdir(d) if re.match(r"output-\d+-\d+$", x))
        if not outs:
            return {"before": before, "after": before, "tried": r["iterations"],
                    "outcome": "no better text in %d iterations" % r["iterations"]}
        with open(os.path.join(d, outs[0][1], "source.c"), encoding="utf-8") as fh:
            cand = permute.splice(text, fh.read(), tg.fn)
        if cand is None:
            return {"before": before, "outcome": "could not splice the permuter's text back"}
        after, _code, err = tg.score(cand)
        if after is None:
            return {"before": before, "outcome": "the permuter's text does not build here: %s" % err}
        return self._keep(job, tg, cand, "permuter score %d after %d iterations" % (outs[0][0], r["iterations"]),
                          before, after, r["iterations"])


def record_match(unit, vaddr, size, text, label, actions=None):
    """Write a person's score-0 version to re/match, verify it from there with match_fn.py,
    then the manifest row and a ledger row.  -> status string."""
    import drive
    import ledger
    import units
    tg = Target(unit, vaddr, size, text)
    cpath = drive.c_path(drive.RE_MATCH, unit, vaddr)
    if os.path.exists(cpath):
        return "re/match already has %s" % drive.rel(cpath)
    header = "/* %s %s  size %s  matched in tools/loop/viewer.py: %s, %s */\n" % (
        unit, vaddr, size, label, datetime.date.today().isoformat())
    os.makedirs(os.path.dirname(cpath), exist_ok=True)
    with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(header + text)
    clean, defsyms, summ = tg.oracle(cpath)
    if not clean:
        os.unlink(cpath)
        log_action(unit, vaddr, "record", label, "", "", "re-verify failed: " + summ, actions)
        return "re-verify failed (%s): nothing recorded" % summ
    extra = []
    if tg.meta["rodata"]:
        extra += ["--rodata", tg.meta["rodata"]]
    syms = drive.syms_args(cpath)
    extra += syms
    for d in (drive.drop_table_defsyms(defsyms, drive.table_syms()) if syms else defsyms):
        extra += ["--defsym", d]
    drive.manifest_append(vaddr, units.resolve(unit).manifest_unit, "%s/%s.c" % (unit, drive.fn_name(vaddr)),
                          size, " ".join(extra))
    if ledger.state(unit, vaddr) != "match":
        ledger.record("match", unit, vaddr, 0, "viewer (%s)" % label)
    log_action(unit, vaddr, "record", label, "", "0", "MATCH", actions)
    return "MATCH recorded: %s, manifest and ledger rows added (commit them; twins.py replay credits the twins)" \
        % drive.rel(cpath)
