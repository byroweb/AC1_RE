#!/usr/bin/env python3
"""viewer.py - watch an overnight pass live in the browser and leave hints for it.

    viewer.py [WORK] [--port 8767] [--replay] [--hints FILE] [--open]

WORK is an overnight.sh work dir (default: the one whose overnight.log changed last,
under $AC1_WORK_ROOT or
~/dev/ac1_work).  The server is stdlib only, listens on 127.0.0.1 and serves one
page (browser-tools/match-viewer/viewer.html) that polls it every 1.5 s:

  GET  /api/state[?t=EPOCH]        stage, active tool, counters, the function list
  GET  /api/func?k=UNIT+VADDR[&src=ID][&t=EPOCH]
                                   one function: its C sources, the oracle's retail |
                                   ours rows for the chosen source, score series, hints
  POST /api/hints                  {unit, vaddr, kind, text}: append to re/match/hints.tsv
  POST /api/hints/delete           {unit, vaddr, kind, text, timestamp}: remove that row
  POST /api/try                    {k, src, move[, minutes][, lines]}: one move on that C,
                                   in the background (tools/loop/human.py); GET /api/jobs?k=
  POST /api/undo, /api/record      {k}: shelve the newest try / record a MATCH try
  POST /api/steer                  {k, what: next|deep|skip|agent|none}: hints for the pipeline
  POST /api/pause                  {on}: <work>/PAUSE holds sweep / permute between bodies

Everything is read from the work dir: overnight.log (the stage), the queue (all1.tsv),
sweep/<unit>_<ADDR>_<variant>.c drafts and sweep_report.tsv, perm1/ perm2/ function
dirs (levers.log, lever.c, permuter.log, output-<score>-<n>/source.c) and
permute_report.tsv, plus the current.json heartbeat sweep.py and permute.py keep
(tools/loop/heartbeat.py): sweep and permute print only their last lines into
overnight.log until a stage ends, so the heartbeat is what makes the view live.

--replay steps through a finished work dir: every event carries a file time (draft
written, permuter dir set up, lever search ended, new permuter best found), and the
page plays them back on a virtual clock (?t=) with play / pause / speed / scrub.

The asm columns are the oracle's own side-by-side (tools/match/match_fn.py through
drive.run_oracle, the auto --defsym rerun of `drive.py try`), run once per distinct C
text in a background thread and cached.
"""
import argparse
import bisect
import datetime
import hashlib
import json
import os
import re
import shutil
import sys
import tempfile
import threading
import time
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))     # hints.py, human.py
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
import hints as hintsmod  # noqa: E402
import human  # noqa: E402

PAGE = os.path.join(HERE, "viewer.html")
TU_MAP = os.path.join(REPO, "tools", "archaeology", "tu_map.tsv")
POLL_STALE = 30.0           # a heartbeat older than this (and not closed) is stale
RATE_WINDOW = 30.0          # compiles/s over the last N seconds

UNIT = r"(?:exe|ovl20[1-4]|fdat\d{3})"
SWEEP_C = re.compile(r"^(%s)_([0-9A-Fa-f]{8})_([\w+-]+)\.c$" % UNIT)
PERM_DIR = re.compile(r"^(%s)_([0-9A-Fa-f]{8})$" % UNIT)
OUTPUT_DIR = re.compile(r"^output-(\d+)-(\d+)$")
STEP = re.compile(r"^== (\d\d):(\d\d):(\d\d) (.*)$")
TRY_MISMATCH = re.compile(r"^MISMATCH@0x([0-9a-f]+)/(\d+)(?:~([0-9.]+))?")
LEVER_LINE = re.compile(r"^\s*(\d+)\s+(\d+)\s+(.*)$")
PERM_BASE = re.compile(r"base score = (\d+)")
PERM_BEST = re.compile(r"new best score! \((\d+) vs")
SWEEP_SUMMARY = re.compile(r"^sweep: (\d+)/(\d+) bodies matched .*?(\d+) tried\)")
PERMUTE_SUMMARY = re.compile(r"^permute: (\d+)/(\d+) matched .*?= ([0-9.]+) compiles/s")
DIFF_ROW = re.compile(r"^([!m ]) ([0-9a-f]{8})  (.*?) \| (.*)$")

# stage kinds in pipeline order, from overnight.sh's `step` titles
STAGES = [("start", "overnight pass"), ("queue", "queue"), ("sweep", "sweep"),
          ("perm1", "permuter, quick"), ("perm2", "permuter, deep"), ("twins", "twins replay"),
          ("verify", "verify"), ("end", None)]
END_TITLES = ("no new matches", "verify did not say", "committed on", "verified, not committed")


# ---------------------------------------------------------------------------
# parsers (pure; tools/match/tests/test_viewer.py)
# ---------------------------------------------------------------------------
def stage_kind(title):
    for p in END_TITLES:
        if title.startswith(p):
            return "end"
    for kind, prefix in STAGES:
        if prefix and title.startswith(prefix):
            return kind
    return "other"


def parse_overnight_log(text, day=None):
    """overnight.log -> {"stages": [{kind, title, clock, t}], "summary": {stage kind: {...}}}.
    day: a datetime.date for the first step (t = local epoch); the clock may pass midnight."""
    stages, summary = [], {}
    prev, offset = None, 0
    kind = None
    for line in text.splitlines():
        m = STEP.match(line)
        if m:
            hms = int(m.group(1)) * 3600 + int(m.group(2)) * 60 + int(m.group(3))
            if prev is not None and hms < prev:
                offset += 1
            prev = hms
            t = None
            if day is not None:
                d = datetime.datetime.combine(day + datetime.timedelta(days=offset), datetime.time())
                t = d.timestamp() + hms
            kind = stage_kind(m.group(4))
            stages.append({"kind": kind, "title": m.group(4), "clock": "%s:%s:%s" % m.group(1, 2, 3),
                           "t": t})
            continue
        m = SWEEP_SUMMARY.match(line)
        if m:
            summary[kind or "sweep"] = {"matched": int(m.group(1)), "total": int(m.group(2)),
                                        "tried": int(m.group(3))}
            continue
        m = PERMUTE_SUMMARY.match(line)
        if m:
            summary[kind or "perm1"] = {"matched": int(m.group(1)), "total": int(m.group(2)),
                                        "rate": float(m.group(3))}
    return {"stages": stages, "summary": summary}


def parse_tries(s):
    """sweep tries column -> [{variant, result, words, closeness, note}]."""
    out = []
    for part in (s or "").split(" | "):
        part = part.strip()
        if not part or part == "-":
            continue
        variant, _sp, rest = part.partition(" ")
        m = TRY_MISMATCH.match(rest)
        result = re.split(r"[@:(]", rest, 1)[0] or rest
        out.append({"variant": variant, "result": result,
                    "words": int(m.group(2)) if m else None,
                    "closeness": float(m.group(3)) if m and m.group(3) else None,
                    "note": rest})
    return out


def parse_levers_log(text):
    """levers.log -> [(eval number, score)] in order."""
    out = []
    for line in text.splitlines():
        m = LEVER_LINE.match(line)
        if m:
            out.append((int(m.group(1)), int(m.group(2))))
    return out


def parse_permuter_log(text):
    """permuter.log -> {"base": int|None, "bests": [scores of each new best, in order]}."""
    base, bests = None, []
    for line in text.splitlines():
        m = PERM_BASE.search(line)
        if m:
            base = int(m.group(1))
            continue
        m = PERM_BEST.search(line)
        if m:
            bests.append(int(m.group(1)))
    return {"base": base, "bests": bests}


def best_so_far(seq):
    out, b = [], None
    for x in seq:
        if x is not None and (b is None or x < b):
            b = x
        out.append(b)
    return out


def thin(seq, n=24):
    """At most n points of seq, first and last kept."""
    if len(seq) <= n:
        return list(seq)
    step = (len(seq) - 1) / (n - 1)
    return [seq[round(i * step)] for i in range(n)]


def parse_oracle_text(text):
    """match_fn.py stdout -> (rows [[mark, addr, retail, ours]], header lines, last line)."""
    rows, head = [], []
    last = ""
    for ln in text.splitlines():
        m = DIFF_ROW.match(ln)
        if m:
            rows.append([m.group(1), "%s" % m.group(2), m.group(3).strip(), m.group(4).strip()])
        elif not rows and ln.strip() and not ln.lstrip().startswith("addr"):
            head.append(ln)
        if ln.strip():
            last = ln.strip()
    return rows, head[:4], last


def read_tsv(path):
    rows = []
    with open(path, encoding="utf-8", errors="replace") as fh:
        hdr = None
        for line in fh:
            line = line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue
            p = line.split("\t")
            if hdr is None:
                hdr = p
                continue
            rows.append(dict(zip(hdr, p)))
    return rows


def load_tu_map(path=TU_MAP):
    """{unit: ([starts], [(start, end, tu, doc)])} for tu_for()."""
    out = {}
    if not os.path.isfile(path):
        return out
    for r in read_tsv(path):
        try:
            s, e = int(r["start"], 16), int(r["end"], 16)
        except (KeyError, ValueError):
            continue
        out.setdefault(r["unit"], []).append((s, e, r.get("tu", ""), r.get("doc_systems", "")))
    return {u: ([x[0] for x in sorted(v)], sorted(v)) for u, v in out.items()}


def tu_for(tumap, unit, vaddr):
    """(tu, doc) of the TU holding vaddr, or ("", "")."""
    if unit not in tumap:
        return "", ""
    starts, spans = tumap[unit]
    v = int(vaddr, 16) if isinstance(vaddr, str) else vaddr
    i = bisect.bisect_right(starts, v) - 1
    if i >= 0 and spans[i][0] <= v < spans[i][1]:
        return spans[i][2], spans[i][3]
    return "", ""


def stamp_day(name):
    """overnight.sh's <stamp> dir name YYYYMMDD-HHMM -> date, or None."""
    m = re.match(r"^(\d{4})(\d\d)(\d\d)-\d{4}", name)
    if not m:
        return None
    try:
        return datetime.date(int(m.group(1)), int(m.group(2)), int(m.group(3)))
    except ValueError:
        return None


def norm_key(unit, vaddr):
    return "%s 0x%08X" % (unit, int(vaddr, 16))


# ---------------------------------------------------------------------------
# the work dir model
# ---------------------------------------------------------------------------
_FILE_CACHE = {}


def read_cached(path, parse):
    """parse(text) of a file, cached by (mtime, size)."""
    try:
        st = os.stat(path)
    except OSError:
        return None
    key = (path, parse)
    sig = (st.st_mtime_ns, st.st_size)
    hit = _FILE_CACHE.get(key)
    if hit and hit[0] == sig:
        return hit[1]
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            val = parse(fh.read())
    except OSError:
        return None
    _FILE_CACHE[key] = (sig, val)
    return val


def mtime(path):
    try:
        return os.stat(path).st_mtime
    except OSError:
        return None


class Func:
    __slots__ = ("unit", "vaddr", "size", "order", "group", "n_members", "drafts", "sweep_row",
                 "perm")

    def __init__(self, unit, vaddr, size=None, order=10 ** 6):
        self.unit, self.vaddr, self.size, self.order = unit, vaddr, size, order
        self.group, self.n_members = "", 1
        self.drafts = []          # [(t, variant, path)]
        self.sweep_row = None
        self.perm = {}            # pass -> dict

    @property
    def key(self):
        return "%s %s" % (self.unit, self.vaddr)


class WorkDir:
    PASSES = ("perm1", "perm2")

    def __init__(self, path):
        self.path = os.path.abspath(path)
        self.name = os.path.basename(self.path.rstrip("\\/"))
        self.lock = threading.Lock()
        self.scanned = 0.0
        self.funcs = {}
        self.log = {"stages": [], "summary": {}}
        self.heartbeats = {}
        self.reports = {}

    # -- scanning ----------------------------------------------------------
    def scan(self, max_age=1.0):
        with self.lock:
            if time.time() - self.scanned < max_age:
                return
            self._scan()
            self.scanned = time.time()

    def _func(self, funcs, unit, vaddr):
        k = norm_key(unit, vaddr)
        f = funcs.get(k)
        if f is None:
            f = funcs[k] = Func(unit, "0x%08X" % int(vaddr, 16))
        return f

    def _scan(self):
        funcs = {}
        day = stamp_day(self.name)
        logp = os.path.join(self.path, "overnight.log")
        if day is None and os.path.exists(logp):
            day = datetime.date.fromtimestamp(os.stat(logp).st_ctime)
        self.log = read_cached(logp, lambda t: parse_overnight_log(t, day)) or {"stages": [], "summary": {}}
        # queue: every all*.tsv (overnight.sh writes all1.tsv)
        try:
            names = sorted(os.listdir(self.path))
        except OSError:
            names = []
        order = 0
        for q in names:
            if not (q.startswith("all") and q.endswith(".tsv")):
                continue
            for r in read_cached(os.path.join(self.path, q), read_tsv_text) or []:
                if "unit" not in r or "vaddr" not in r:
                    continue
                try:
                    f = self._func(funcs, r["unit"], r["vaddr"])
                except ValueError:
                    continue
                f.order = min(f.order, order)
                order += 1
                f.size = int(r.get("size") or 0) or f.size
                f.group, f.n_members = r.get("group", ""), int(r.get("n_members") or 1)
        # sweep drafts and report
        sd = os.path.join(self.path, "sweep")
        if os.path.isdir(sd):
            with os.scandir(sd) as it:
                for e in it:
                    m = SWEEP_C.match(e.name)
                    if m and e.is_file():
                        f = self._func(funcs, m.group(1), m.group(2))
                        f.drafts.append((e.stat().st_mtime, m.group(3), e.path))
            for r in read_cached(os.path.join(sd, "sweep_report.tsv"), read_tsv_text) or []:
                try:
                    f = self._func(funcs, r["unit"], r["vaddr"])
                except (KeyError, ValueError):
                    continue
                f.sweep_row = r
                f.size = f.size or int(r.get("size") or 0)
            self.reports["sweep"] = mtime(os.path.join(sd, "sweep_report.tsv"))
        for f in funcs.values():
            f.drafts.sort()
        # permuter passes
        for ps in self.PASSES:
            pd = os.path.join(self.path, ps)
            if not os.path.isdir(pd):
                continue
            rep = {}
            for r in read_cached(os.path.join(pd, "permute_report.tsv"), read_tsv_text) or []:
                try:
                    rep[norm_key(r["unit"], r["vaddr"])] = r
                except (KeyError, ValueError):
                    pass
            self.reports[ps] = mtime(os.path.join(pd, "permute_report.tsv"))
            with os.scandir(pd) as it:
                for e in it:
                    m = PERM_DIR.match(e.name)
                    if not (m and e.is_dir()):
                        continue
                    f = self._func(funcs, m.group(1), m.group(2))
                    f.perm[ps] = self._perm_entry(e.path, rep.get(f.key))
        self.heartbeats = {}
        for sub in ("sweep",) + self.PASSES:
            hb = read_cached(os.path.join(self.path, sub, "current.json"), json_or_none)
            if hb:
                self.heartbeats[sub] = hb
        self.funcs = funcs

    @staticmethod
    def _perm_entry(d, report_row):
        e = {"dir": d, "report": report_row, "outputs": []}
        e["t_start"] = mtime(os.path.join(d, "meta.json")) or mtime(os.path.join(d, "draft.c"))
        e["t_levers"] = mtime(os.path.join(d, "levers.log"))
        e["t_perm"] = mtime(os.path.join(d, "permuter.log"))
        e["levers"] = read_cached(os.path.join(d, "levers.log"), parse_levers_log) or []
        e["permuter"] = read_cached(os.path.join(d, "permuter.log"), parse_permuter_log)
        try:
            with os.scandir(d) as it:
                for x in it:
                    m = OUTPUT_DIR.match(x.name)
                    if m and x.is_dir():
                        e["outputs"].append((x.stat().st_mtime, int(m.group(1)), x.path))
        except OSError:
            pass
        e["outputs"].sort()
        ends = [t for t in (e["t_levers"], e["t_perm"], mtime(os.path.join(d, "result.c"))) if t]
        e["t_end"] = max(ends) if ends else e["t_start"]
        return e

    # -- time range for replay ---------------------------------------------
    def time_range(self):
        ts = [s["t"] for s in self.log["stages"] if s["t"]]
        for f in self.funcs.values():
            ts += [d[0] for d in f.drafts]
            for e in f.perm.values():
                ts += [t for t in (e["t_start"], e["t_end"]) if t]
        return (min(ts), max(ts)) if ts else (time.time(), time.time())

    # -- per-function state at time t (None = now) --------------------------
    def func_state(self, f, t=None, live_done=None):
        """{status, best, spark, tried, events} for f as of t."""
        upto = (lambda x: x is not None and x <= t) if t is not None else (lambda x: x is not None)
        drafts = [d for d in f.drafts if upto(d[0])]
        spark, status, best = [], "open", None
        tries = parse_tries(f.sweep_row.get("tries")) if f.sweep_row else []
        last_draft = f.drafts[-1][0] if f.drafts else None
        sweep_known = f.sweep_row is not None and (t is None or upto(last_draft))
        hb = (live_done or {}).get("sweep", {}).get(f.key)
        if drafts:
            status = "tried"
        if sweep_known:
            for tr in tries:
                if tr["words"] is not None:
                    spark.append(tr["words"])
            if f.sweep_row.get("status") == "MATCH":
                status = "MATCH"
        elif hb and t is None:          # live, before sweep_report.tsv exists
            if hb.get("words") is not None:
                spark.append(hb["words"])
            status = "tried"
        for ps in self.PASSES:
            e = f.perm.get(ps)
            if not e or not upto(e["t_start"]):
                continue
            if status != "MATCH":
                status = "near"
            n = None
            if t is not None and e["t_levers"] and t < e["t_levers"]:
                frac = (t - e["t_start"]) / max(1e-6, e["t_levers"] - e["t_start"])
                n = max(0, int(len(e["levers"]) * frac))
            spark += [x for x in self.lever_series(f, e, n) if x is not None][-12:]
            rep = e["report"]
            if rep is not None and (t is None or upto(e["t_end"])) and rep.get("status") == "MATCH":
                status = "MATCH"
        if t is None and any(d.get(f.key, {}).get("status") == "MATCH" for d in (live_done or {}).values()):
            status = "MATCH"
        spark = [x for x in spark if x is not None]
        best = min(spark) if spark else None
        return {"status": status, "best": best, "spark": thin(spark, 20), "tried": bool(drafts) or sweep_known
                or bool(hb)}

    def last_event(self, t=None):
        """(time, key, what) of the newest event at or before t."""
        best = None
        for f in self.funcs.values():
            for d in f.drafts:
                if (t is None or d[0] <= t) and (best is None or d[0] > best[0]):
                    best = (d[0], f.key, "sweep " + d[1])
            for ps, e in f.perm.items():
                for tt, what in ((e["t_start"], "levers"), (e["t_levers"], "permuter")):
                    if tt and (t is None or tt <= t) and (best is None or tt > best[0]):
                        best = (tt, f.key, what)
        return best

    @staticmethod
    def lever_series(f, e, upto=None):
        """Best-so-far lever scores of permuter entry e, starting from the draft's own score
        (levers.log lists only the children): the report's base, else the sweep's words."""
        base = None
        rep = e["report"] or {}
        if (rep.get("base") or "").isdigit():
            base = int(rep["base"])
        elif f.sweep_row:
            w = [tr["words"] for tr in parse_tries(f.sweep_row.get("tries")) if tr["words"] is not None]
            base = min(w) if w else None
        lv = [s for _n, s in e["levers"]]
        if upto is not None:
            lv = lv[:upto]
        return best_so_far([base] + lv) if base is not None else best_so_far(lv)

    def phase_at(self, e, t):
        """levers / permuter / done for a permuter entry at t (replay)."""
        if not e["t_start"] or t < e["t_start"]:
            return None
        if e["t_end"] and t >= e["t_end"]:
            return "done"
        if e["t_levers"] and t >= e["t_levers"]:
            return "permuter"
        return "levers"

    def compile_rate(self, t):
        """Estimated compiles/s over [t - RATE_WINDOW, t] from file times: one per sweep
        draft, lever evaluations spread over the lever stage, permuter iterations over the
        permuter stage."""
        lo, n = t - RATE_WINDOW, 0.0
        for f in self.funcs.values():
            n += sum(1 for d in f.drafts if lo < d[0] <= t)
            for e in f.perm.values():
                rep = e["report"] or {}
                spans = []
                if e["t_start"] and e["t_levers"]:
                    spans.append((e["t_start"], e["t_levers"], len(e["levers"]) or int(rep.get("lever_evals") or 0)))
                if e["t_levers"] and e["t_end"] and rep.get("iterations"):
                    spans.append((e["t_levers"], e["t_end"], int(rep["iterations"])))
                for a, b, cnt in spans:
                    if b <= a or cnt <= 0:
                        continue
                    ov = min(b, t) - max(a, lo)
                    if ov > 0:
                        n += cnt * ov / (b - a)
        return n / RATE_WINDOW


def read_tsv_text(text):
    rows, hdr = [], None
    for line in text.splitlines():
        if not line or line.startswith("#"):
            continue
        p = line.split("\t")
        if hdr is None:
            hdr = p
            continue
        rows.append(dict(zip(hdr, p)))
    return rows


def json_or_none(text):
    try:
        return json.loads(text)
    except ValueError:
        return None


# ---------------------------------------------------------------------------
# the oracle, in the background
# ---------------------------------------------------------------------------
class Oracle:
    """Runs match_fn.py on C texts in one worker thread; results cached by text hash."""

    def __init__(self):
        self.cache, self.pending = {}, []
        self.cond = threading.Condition()
        self.tmp = tempfile.mkdtemp(prefix="ac1_viewer_")
        self.rows = None
        threading.Thread(target=self._loop, daemon=True).start()

    def key(self, unit, vaddr, text):
        return hashlib.sha1(("%s %s\n" % (unit, vaddr) + text).encode("utf-8", "replace")).hexdigest()

    def get(self, unit, vaddr, size, text):
        k = self.key(unit, vaddr, text)
        with self.cond:
            if k in self.cache:
                return self.cache[k]
            if k not in [p[0] for p in self.pending]:
                self.pending.insert(0, (k, unit, vaddr, size, text))   # newest request first
                del self.pending[4:]
                self.cond.notify()
        return {"pending": True}

    def _loop(self):
        while True:
            with self.cond:
                while not self.pending:
                    self.cond.wait()
                job = self.pending.pop(0)
            try:
                res = self.run(*job[1:])
            except Exception as e:      # noqa: BLE001 - shown in the page, never fatal
                res = {"result": "ERROR", "error": "%s: %s" % (type(e).__name__, e), "rows": []}
            with self.cond:
                self.cache[job[0]] = res

    def _rodata(self, unit, vaddr):
        import drive
        if self.rows is None:
            try:
                self.rows = drive.load_rows()
            except Exception:           # noqa: BLE001
                self.rows = {}
        r = self.rows.get((unit, vaddr))
        if r and int(r.get("jump_tables") or 0) and r.get("jt_addrs"):
            return r["jt_addrs"].split(",")[0].strip()
        return None

    @staticmethod
    def _lines(keep, text, fn):
        """{offset: C line} of our object (cc1's .loc lines through objdump -l), or {}."""
        import subprocess
        import units
        obj, fni = os.path.join(keep, "fn.o"), os.path.join(keep, "fn.i")
        if not (os.path.isfile(obj) and os.path.isfile(fni)):
            return {}
        try:
            dump = subprocess.run([units.find_objdump(), "-dl", obj], capture_output=True, text=True,
                                  errors="replace").stdout
            with open(fni, encoding="utf-8", errors="replace") as fh:
                return human.line_map(dump, fh.read(), text, fn)
        except Exception:               # noqa: BLE001 - the line map is a nicety
            return {}

    def run(self, unit, vaddr, size, text):
        import drive
        import units
        u = units.resolve(unit)
        va = int(vaddr, 16)
        cpath = os.path.join(self.tmp, "%s_%08X_%s.c" % (unit, va, self.key(unit, vaddr, text)[:8]))
        with open(cpath, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        rodata = self._rodata(unit, vaddr)
        keep = cpath[:-2] + ".keep"
        lines = {}
        try:
            argv = drive.oracle_argv(u, va, cpath, size, rodata, [], keep=keep)
            rc, facts, out, err = drive.run_oracle(argv)
            auto = ["%s=0x%s" % (s, drive.AUTO_SYM.match(s).group(2).upper())
                    for s in (facts or {}).get("unresolved_symbols") or [] if drive.AUTO_SYM.match(s)]
            if auto:
                argv = drive.oracle_argv(u, va, cpath, size, rodata, auto, keep=keep)
                rc, facts, out, err = drive.run_oracle(argv)
            lines = self._lines(keep, text, "func_%08X" % va)
        finally:
            try:
                os.unlink(cpath)
            except OSError:
                pass
            shutil.rmtree(keep, ignore_errors=True)
        rows, head, last = parse_oracle_text(out)
        facts = facts or {}
        res = {"result": facts.get("result") or "ERROR", "diff_words": facts.get("diff_words"),
               "aligned": facts.get("aligned_words"), "bytes": facts.get("bytes"), "size": size,
               "first_diff": facts.get("first_diff_offset"), "masked": facts.get("masked_words"),
               "rows": rows, "head": head, "last": last, "rc": rc,
               "lines": {"%08x" % (va + off): ln for off, ln in lines.items()}}
        if res["result"] == "ERROR":
            res["error"] = facts.get("error") or (err.strip().splitlines() or [last])[-1]
            res["rows"] = retail_rows(u, va, size)
        return res


def retail_rows(u, vaddr, size):
    """Retail listing alone (when our C does not build): [[" ", addr, asm, ""]]."""
    import units
    import subprocess
    try:
        objdump = units.find_objdump()
        cmd = [objdump, "-D", "-b", "binary", "-m", "mips:3000", "-EL", "--adjust-vma=0x%X" % u.base,
               "--start-address=0x%X" % vaddr, "--stop-address=0x%X" % (vaddr + size), u.image_path]
        out = subprocess.run(cmd, capture_output=True, text=True, errors="replace").stdout
    except Exception:                   # noqa: BLE001
        return []
    rows = []
    for ln in out.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\s+[0-9a-f]{8}\s+(\S+)\s*(.*)$", ln)
        if m:
            rows.append([" ", "%08x" % int(m.group(1), 16), (m.group(2) + " " + m.group(3).replace(" ", "")).strip(),
                         ""])
    return rows


# ---------------------------------------------------------------------------
# the app: state, function detail, hints
# ---------------------------------------------------------------------------
class App:
    def __init__(self, work, hints_path=None, replay=False, oracle=True, actions_path=None):
        self.wd = WorkDir(work)
        self.hints_path = hints_path or hintsmod.HINTS
        self.actions_path = actions_path or human.ACTIONS
        self.tries = human.Tries(os.path.join(self.wd.path, "human"), self.actions_path)
        self.replay = replay
        self.tumap = load_tu_map()
        self.oracle = Oracle() if oracle else None
        self.rate_hist = []           # live: (time, compiles from the open heartbeats)
        self.hints_lock = threading.Lock()

    # -- helpers -----------------------------------------------------------
    def live_done(self):
        return {k: hb.get("done", {}) for k, hb in self.wd.heartbeats.items()}

    def open_heartbeats(self, now):
        return {k: hb for k, hb in self.wd.heartbeats.items()
                if not hb.get("closed") and now - hb.get("t", 0) < POLL_STALE}

    def hints_rows(self):
        return hintsmod.load(self.hints_path)

    # -- /api/state ----------------------------------------------------------
    def state(self, t=None):
        wd = self.wd
        wd.scan(max_age=5.0 if self.replay else 1.0)
        now = time.time()
        t0, t1 = wd.time_range()
        if not self.replay:
            t = None
        at = t if t is not None else now
        stages = [s for s in wd.log["stages"] if t is None or (s["t"] is not None and s["t"] <= t)]
        stage = stages[-1] if stages else None
        live = self.open_heartbeats(now) if t is None else {}
        live_done = self.live_done() if t is None else {}
        hrows = self.hints_rows()
        flags = hintsmod.triage(hrows)
        nhints = {}
        for r in hrows:
            if r["kind"] != "triage":
                k = "%s %s" % (r["unit"], r["vaddr"])
                nhints[k] = nhints.get(k, 0) + 1
        funcs, counts = [], {"open": 0, "tried": 0, "near": 0, "MATCH": 0}
        for f in sorted(wd.funcs.values(), key=lambda f: (f.order, f.unit, f.vaddr)):
            st = wd.func_state(f, t, live_done)
            counts[st["status"]] = counts.get(st["status"], 0) + 1
            tu, doc = tu_for(self.tumap, f.unit, f.vaddr)
            funcs.append({"k": f.key, "unit": f.unit, "vaddr": f.vaddr, "size": f.size, "tu": tu, "doc": doc,
                          "status": st["status"], "best": st["best"], "spark": st["spark"],
                          "members": f.n_members, "triage": flags.get((f.unit, f.vaddr)),
                          "hints": nhints.get(f.key, 0)})
        # hinted functions outside this run still get a row, so their notes stay reachable
        have = {x["k"] for x in funcs}
        for (u, v), fl in flags.items():
            k = "%s %s" % (u, v)
            if k not in have:
                have.add(k)
                funcs.append({"k": k, "unit": u, "vaddr": v, "size": None, "tu": tu_for(self.tumap, u, v)[0],
                              "doc": "", "status": "open", "best": None, "spark": [], "members": 1,
                              "triage": fl, "hints": nhints.get(k, 0), "outside": True})
        # active tool and current function
        active, tool, variant = [], None, None
        kind = stage["kind"] if stage else None
        if live:
            for sub, hb in live.items():
                for k, a in hb.get("active", {}).items():
                    active.append({"k": k, "phase": a.get("phase"), "since": a.get("since"), "pass": sub,
                                   "iterations": a.get("iterations"), "lever": a.get("lever"),
                                   "perm_best": a.get("perm_best")})
        elif t is not None:
            for f in wd.funcs.values():
                for ps, e in f.perm.items():
                    ph = wd.phase_at(e, t)
                    if ph in ("levers", "permuter"):
                        active.append({"k": f.key, "phase": ph, "since": e["t_start"], "pass": ps})
                if kind == "sweep":
                    cur = [d for d in f.drafts if d[0] <= t and t - d[0] < 3.0]
                    if cur:
                        active.append({"k": f.key, "phase": cur[-1][1], "since": cur[-1][0], "pass": "sweep"})
        active.sort(key=lambda a: -(a.get("since") or 0))
        phases = {}
        for a in active:
            p = a["phase"] or "-"
            phases[p] = phases.get(p, 0) + 1
        if kind == "sweep":
            tool = "sweep"
            sv = [a["phase"] for a in active if a["pass"] == "sweep"]
            variant = max(set(sv), key=sv.count) if sv else None
            if variant is None:
                ev = wd.last_event(t)
                if ev and ev[2].startswith("sweep "):
                    variant = ev[2][6:]
        elif kind in ("perm1", "perm2"):
            tool = "permuter" if phases.get("permuter", 0) >= phases.get("levers", 0) and phases.get("permuter") \
                else "levers"
        elif kind in ("twins", "verify"):
            tool = kind
        current = active[0]["k"] if active else None
        if current is None:
            ev = wd.last_event(t)
            current = ev[1] if ev else None
        # counters
        if live:
            # every heartbeat of the run, closed ones too: a stage ending must not read as a drop
            total_c = sum(hb.get("compiles", 0) for hb in wd.heartbeats.values())
            if self.rate_hist and total_c < self.rate_hist[-1][1]:
                self.rate_hist = []         # a heartbeat was restarted (a new pass in the same dir)
            self.rate_hist = [x for x in self.rate_hist if now - x[0] < RATE_WINDOW] + [(now, total_c)]
            a, b = self.rate_hist[0], self.rate_hist[-1]
            rate = (b[1] - a[1]) / (b[0] - a[0]) if b[0] - a[0] > 2 else None
            if rate is None:
                hb = max(live.values(), key=lambda h: h.get("t", 0))
                rate = hb.get("compiles", 0) / max(1.0, hb.get("t", now) - hb.get("started", now))
        else:
            rate = wd.compile_rate(at)
        hb_info = {}
        for sub, hb in wd.heartbeats.items():
            hb_info[sub] = {"tool": hb.get("tool"), "label": hb.get("label"), "total": hb.get("total"),
                            "finished": hb.get("finished"), "matches": hb.get("matches"),
                            "compiles": hb.get("compiles"), "closed": hb.get("closed"),
                            "stale": not hb.get("closed") and now - hb.get("t", 0) >= POLL_STALE}
        start = next((s["t"] for s in wd.log["stages"] if s["t"]), t0)
        paused = os.path.exists(os.path.join(wd.path, "PAUSE"))
        return {"mode": "replay" if self.replay else "live", "work": wd.name, "t": at, "range": [t0, t1],
                "paused": paused,
                "jobs": sum(1 for j in self.tries.list() if j["status"] in ("queued", "running")),
                "stage": stage, "stages": wd.log["stages"], "summary": wd.log["summary"],
                "tool": tool, "variant": variant, "phases": phases, "active": active[:40],
                "current": current, "heartbeats": hb_info,
                "counters": {"total": len(wd.funcs), "tried": sum(1 for x in funcs if x["status"] != "open"),
                             "near": counts.get("near", 0), "matches": counts.get("MATCH", 0),
                             "rate": rate, "elapsed": at - start if start else None},
                "funcs": funcs}

    # -- /api/func -----------------------------------------------------------
    def sources(self, f, t=None):
        """[{id, label, t, score, kind}] of the C texts this run has for f (at t)."""
        out = []
        upto = (lambda x: x is not None and x <= t) if t is not None else (lambda x: x is not None)
        words = {}
        if f.sweep_row:
            for tr in parse_tries(f.sweep_row.get("tries")):
                words[tr["variant"]] = tr["words"]
        for tt, variant, path in f.drafts:
            if upto(tt):
                out.append({"id": "sweep:" + variant, "label": "sweep " + variant, "t": tt,
                            "score": words.get(variant), "unit": "words", "path": path})
        for ps in WorkDir.PASSES:
            e = f.perm.get(ps)
            if not e or not upto(e["t_start"]):
                continue
            tag = "quick" if ps == "perm1" else "deep"
            lv = os.path.join(e["dir"], "lever.c")
            lvb = self.wd.lever_series(f, e)
            if os.path.exists(lv) and upto(mtime(lv)):
                out.append({"id": ps + ":lever", "label": "levers (%s)" % tag, "t": mtime(lv),
                            "score": lvb[-1] if lvb else None, "unit": "words", "path": lv})
            elif os.path.exists(os.path.join(e["dir"], "draft.c")):
                out.append({"id": ps + ":draft", "label": "draft (%s)" % tag, "t": e["t_start"],
                            "score": lvb[0] if lvb else None, "unit": "words",
                            "path": os.path.join(e["dir"], "draft.c")})
            outs = [o for o in e["outputs"] if upto(o[0])]
            if outs:
                o = min(outs, key=lambda o: (o[1], -o[0]))
                out.append({"id": ps + ":perm", "label": "permuter best (%s)" % tag, "t": o[0], "score": o[1],
                            "unit": "permuter score", "path": os.path.join(o[2], "source.c"),
                            "splice": True, "dir": e["dir"]})
            res = os.path.join(e["dir"], "result.c")
            if os.path.exists(res) and upto(mtime(res)):
                out.append({"id": ps + ":result", "label": "score-0 result (%s)" % tag, "t": mtime(res),
                            "score": 0, "unit": "permuter score", "path": res})
        for v in self.tries.versions(f.unit, f.vaddr).list():
            out.append({"id": "human:%d" % v["n"], "label": "your try %d" % v["n"], "t": v["t"],
                        "score": v["score"], "unit": "words", "path": v["path"], "note": v["label"],
                        "result": v["result"]})
        return out

    def default_source(self, f, srcs):
        mine = [s for s in srcs if s["id"].startswith("human:")]
        if mine:                        # the person is working on it: their newest try
            return mine[-1]
        perm = [s for s in srcs if not s["id"].startswith(("sweep:", "human:"))]
        if perm:
            return max(perm, key=lambda s: s["t"])
        scored = [s for s in srcs if s["score"] is not None]
        if scored:                      # fewest differing words; the report's closest on a tie
            best = f.sweep_row.get("best_variant") if f.sweep_row else None
            return min(scored, key=lambda s: (s["score"], s["id"] != "sweep:%s" % best))
        return srcs[-1] if srcs else None

    def source_text(self, s):
        with open(s["path"], encoding="utf-8", errors="replace") as fh:
            text = fh.read()
        if s.get("splice"):
            import permute
            d = s["dir"]
            fn = "func_%s" % os.path.basename(d).split("_")[1].upper()
            for base in ("lever.c", "draft.c"):
                p = os.path.join(d, base)
                if os.path.exists(p):
                    with open(p, encoding="utf-8", errors="replace") as fh:
                        sp = permute.splice(fh.read(), text, fn)
                    if sp:
                        return sp
        return text

    def func(self, key, src=None, t=None):
        wd = self.wd
        wd.scan(max_age=5.0 if self.replay else 1.0)
        if not self.replay:
            t = None
        unit, _sp, vaddr = key.partition(" ")
        try:
            k = norm_key(unit, vaddr)
        except ValueError:
            return {"error": "bad function key %r" % key}
        f = wd.funcs.get(k)
        if f is None:
            f = Func(unit, "0x%08X" % int(vaddr, 16))
        srcs = self.sources(f, t)
        pick = next((s for s in srcs if s["id"] == src), None) or self.default_source(f, srcs)
        text, asm = None, None
        if pick:
            try:
                text = self.source_text(pick)
            except OSError as e:
                text = "/* %s */" % e
            size = f.size or self._size(f)
            if self.oracle and size:
                asm = self.oracle.get(f.unit, f.vaddr, size, text)
        tu, doc = tu_for(self.tumap, f.unit, f.vaddr)
        series = {"sweep": [], "passes": {}}
        if f.sweep_row:
            series["sweep"] = [[tr["variant"], tr["words"], tr["result"]] for tr in parse_tries(f.sweep_row.get("tries"))]
        for ps, e in f.perm.items():
            p = e["permuter"] or {"base": None, "bests": []}
            outs = [[o[0], o[1]] for o in e["outputs"] if t is None or o[0] <= t]
            series["passes"][ps] = {"levers": wd.lever_series(f, e),
                                    "lever_evals": len(e["levers"]), "permuter_base": p["base"],
                                    "permuter": outs or [[None, x] for x in p["bests"]],
                                    "report": e["report"], "t_start": e["t_start"], "t_end": e["t_end"]}
        st = wd.func_state(f, t, self.live_done() if t is None else {})
        symptoms, explanation = [], None
        if asm and not asm.get("pending"):
            symptoms = human.classify(asm.get("rows") or [], f.size, asm.get("bytes"))
            explanation = human.explain(symptoms, asm.get("result"), asm.get("diff_words"), f.size,
                                        asm.get("bytes"))
        rows = hintsmod.for_function(f.unit, f.vaddr, self.hints_rows())
        tri = None
        for r in rows:
            if r["kind"] == "triage":
                tri = None if r["text"] == "none" else r["text"]
        return {"k": f.key, "unit": f.unit, "vaddr": f.vaddr, "size": f.size, "tu": tu, "doc": doc,
                "members": f.n_members, "group": f.group, "status": st["status"], "best": st["best"],
                "sources": [{k2: v for k2, v in s.items() if k2 not in ("path", "dir")} for s in srcs],
                "source": pick["id"] if pick else None, "text": text, "asm": asm, "series": series,
                "sweep": f.sweep_row, "hints": rows, "triage": tri,
                "steer": next((r["text"] for r in reversed(rows) if r["kind"] == "steer"), None),
                "symptoms": symptoms, "explain": explanation, "jobs": self.tries.list(f.key)[:8],
                "moves": [{"id": k, "label": v[0], "why": v[1]} for k, v in human.MOVES.items()]}

    def _size(self, f):
        try:
            import drive
            r = drive.load_rows().get((f.unit, f.vaddr))
            return int(r["size"]) if r else None
        except Exception:              # noqa: BLE001
            return None

    # -- hints ---------------------------------------------------------------
    def add_hint(self, body):
        with self.hints_lock:
            row = hintsmod.append(body.get("unit", ""), body.get("vaddr", ""), body.get("kind", ""),
                                  body.get("text", ""), self.hints_path)
        return {"ok": True, "row": row, "hints": hintsmod.for_function(row["unit"], row["vaddr"], self.hints_rows())}

    def _body(self, body):
        """-> (Func, sources, picked source) for body["k"] / body["src"]."""
        unit, _sp, vaddr = str(body.get("k", "")).partition(" ")
        k = norm_key(unit, vaddr)
        self.wd.scan(max_age=1.0)
        f = self.wd.funcs.get(k) or Func(unit, "0x%08X" % int(vaddr, 16))
        f.size = f.size or self._size(f)
        srcs = self.sources(f)
        pick = next((s for s in srcs if s["id"] == body.get("src")), None) or self.default_source(f, srcs)
        return f, srcs, pick

    def try_move(self, body):
        f, _srcs, pick = self._body(body)
        if not pick:
            raise ValueError("no C for this body yet")
        if not f.size:
            raise ValueError("unknown size for %s" % f.key)
        lines = body.get("lines")
        job = self.tries.submit(f.unit, f.vaddr, f.size, self.source_text(pick), pick["id"], body.get("move", ""),
                                body.get("minutes"), [int(x) for x in lines] if lines else None)
        return {"ok": True, "job": job}

    def undo(self, body):
        f, _s, _p = self._body(body)
        v = self.tries.versions(f.unit, f.vaddr).undo()
        if v:
            human.log_action(f.unit, f.vaddr, "undo", "your try %d (%s)" % (v["n"], v["label"]), v["score"],
                             "", "", self.actions_path)
        return {"ok": bool(v), "undone": v and v["n"]}

    def record(self, body):
        f, srcs, _p = self._body(body)
        mine = [s for s in srcs if s["id"].startswith("human:") and s.get("result") == "MATCH"]
        pick = next((s for s in mine if s["id"] == body.get("src")), None) or (mine[-1] if mine else None)
        if not pick:
            raise ValueError("no try of yours is a MATCH yet")
        with open(pick["path"], encoding="utf-8") as fh:
            text = fh.read()
        return {"ok": True, "status": human.record_match(f.unit, f.vaddr, f.size, text, pick["note"],
                                                         self.actions_path)}

    STEER = {"next": [("triage", "priority")], "skip": [("triage", "skip")], "deep": [("steer", "deep")],
             "agent": [("steer", "agent"), ("triage", "priority")],
             "none": [("steer", "none"), ("triage", "none")]}

    def steer(self, body):
        what = body.get("what")
        if what not in self.STEER:
            raise ValueError("steer what: %s" % ", ".join(self.STEER))
        unit, _sp, vaddr = str(body.get("k", "")).partition(" ")
        with self.hints_lock:
            for kind, text in self.STEER[what]:
                hintsmod.append(unit, vaddr, kind, text, self.hints_path)
        human.log_action(hintsmod.norm_unit(unit), hintsmod.norm_vaddr(vaddr), "steer", what, "", "", "",
                         self.actions_path)
        return {"ok": True, "hints": hintsmod.for_function(unit, vaddr, self.hints_rows())}

    def pause(self, body):
        p = os.path.join(self.wd.path, "PAUSE")
        if body.get("on"):
            with open(p, "w", encoding="utf-8") as fh:
                fh.write("paused from browser-tools/match-viewer/viewer.py %s\n" % human.now())
        elif os.path.exists(p):
            os.unlink(p)
        human.log_action("", "", "pause" if body.get("on") else "resume", self.wd.name, "", "", "",
                         self.actions_path)
        return {"ok": True, "paused": os.path.exists(p)}

    def delete_hint(self, body):
        with self.hints_lock:
            ok = hintsmod.delete(body, self.hints_path)
        return {"ok": ok, "hints": hintsmod.for_function(body["unit"], body["vaddr"], self.hints_rows())}


# ---------------------------------------------------------------------------
# HTTP
# ---------------------------------------------------------------------------
def make_handler(app):
    class Handler(BaseHTTPRequestHandler):
        server_version = "ac1-viewer/1"

        def log_message(self, fmt, *a):      # quiet: the page polls every 1.5 s
            pass

        def send(self, code, body, ctype="application/json; charset=utf-8"):
            data = body if isinstance(body, bytes) else json.dumps(body).encode("utf-8")
            self.send_response(code)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(data)

        def do_GET(self):
            u = urllib.parse.urlparse(self.path)
            q = {k: v[0] for k, v in urllib.parse.parse_qs(u.query).items()}
            t = float(q["t"]) if q.get("t") else None
            try:
                if u.path in ("/", "/index.html"):
                    with open(PAGE, "rb") as fh:
                        return self.send(200, fh.read(), "text/html; charset=utf-8")
                if u.path == "/api/state":
                    return self.send(200, app.state(t))
                if u.path == "/api/func":
                    return self.send(200, app.func(q.get("k", ""), q.get("src"), t))
                if u.path == "/api/jobs":
                    return self.send(200, {"jobs": app.tries.list(q.get("k"))})
                if u.path == "/api/hints":
                    unit, _s, vaddr = q.get("k", "").partition(" ")
                    return self.send(200, {"hints": hintsmod.for_function(unit, vaddr, app.hints_rows())})
            except (ValueError, OSError) as e:
                return self.send(400, {"error": str(e)})
            self.send(404, {"error": "not found"})

        def do_POST(self):
            # same-origin only: a page on another site must not write hints
            origin = self.headers.get("Origin")
            if origin and urllib.parse.urlparse(origin).netloc != self.headers.get("Host"):
                return self.send(403, {"error": "cross-origin request refused"})
            if "json" not in (self.headers.get("Content-Type") or ""):
                return self.send(415, {"error": "send application/json"})
            try:
                n = int(self.headers.get("Content-Length") or 0)
                body = json.loads(self.rfile.read(min(n, 65536)).decode("utf-8"))
                if not isinstance(body, dict):
                    raise ValueError("want a JSON object")
                if self.path == "/api/hints":
                    return self.send(200, app.add_hint(body))
                if self.path == "/api/hints/delete":
                    return self.send(200, app.delete_hint(body))
                route = {"/api/try": app.try_move, "/api/undo": app.undo, "/api/record": app.record,
                         "/api/steer": app.steer, "/api/pause": app.pause}.get(self.path)
                if route:
                    return self.send(200, route(body))
            except (ValueError, KeyError, OSError) as e:
                return self.send(400, {"error": str(e)})
            self.send(404, {"error": "not found"})
    return Handler


class Server(ThreadingHTTPServer):
    # SO_REUSEADDR on Windows lets a second server bind a port that is in use
    allow_reuse_address = os.name != "nt"
    daemon_threads = True


def already_running(url):
    """True when a viewer already answers at url."""
    import urllib.request
    try:
        with urllib.request.urlopen(url + "api/state?probe=1", timeout=1.5) as r:
            return "mode" in json.load(r)
    except (OSError, ValueError):
        return False


def default_work():
    root = os.environ.get("AC1_WORK_ROOT") or os.path.join(os.path.expanduser("~"), "dev", "ac1_work")
    try:
        dirs = [os.path.join(root, d) for d in os.listdir(root)]
    except OSError:
        dirs = []
    runs = [d for d in dirs if os.path.isfile(os.path.join(d, "overnight.log"))]   # not other tools' dirs
    return max(runs, key=lambda d: os.stat(os.path.join(d, "overnight.log")).st_mtime) if runs else None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("work", nargs="?", help="overnight.sh work dir (default: the one under ~/dev/ac1_work "
                    "whose overnight.log changed last)")
    ap.add_argument("--port", type=int, default=8767)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--replay", action="store_true", help="play a finished work dir back on a virtual clock")
    ap.add_argument("--hints", default=hintsmod.HINTS, help="hints file (default re/match/hints.tsv)")
    ap.add_argument("--actions", default=human.ACTIONS, help="action log (default re/match/actions.tsv)")
    ap.add_argument("--no-oracle", action="store_true", help="do not compile the C (no asm columns)")
    ap.add_argument("--open", action="store_true", help="open the page in the default browser")
    args = ap.parse_args(argv)
    work = args.work or default_work()
    if not work or not os.path.isdir(work):
        ap.error("no work dir (pass one, or run tools/loop/overnight.sh first)")
    if args.host not in ("127.0.0.1", "localhost", "::1"):
        ap.error("the viewer runs compiles on request: it listens on 127.0.0.1 only")
    url = "http://%s:%d/" % ("localhost" if args.host in ("127.0.0.1", "0.0.0.0") else args.host, args.port)
    if already_running(url):            # a second double-click: show the running one
        print("viewer: already running on %s" % url)
        if args.open:
            import webbrowser
            webbrowser.open(url)
        return 0
    app = App(work, args.hints, args.replay, oracle=not args.no_oracle, actions_path=args.actions)
    try:
        srv = Server((args.host, args.port), make_handler(app))
    except OSError as e:
        ap.error("port %d is taken (%s): pass --port N" % (args.port, e))
    print("viewer: %s %s on %s (Ctrl+C stops)" % ("replaying" if args.replay else "watching", work, url))
    sys.stdout.flush()
    if args.open:
        import webbrowser
        webbrowser.open(url)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        if app.oracle:
            shutil.rmtree(app.oracle.tmp, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
