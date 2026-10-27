#!/usr/bin/env python3
"""coverage_tag.py - tag functions with the game screen they run in, while you play.

    coverage_tag.py [--unit all|front|mission|vs|<unit>]... [--set all|unnamed|unseen|open]
                    [--context garage] [--hits 1] [--max-contexts 8] [--max-armed N]
                    [--capture UNIT:VADDR|NAME]... [--minutes N]
    coverage_tag.py --check            MCP health check + which units are loaded, then exit

Start DuckStation (the MCP fork, port 2346) with the game running, start this script
in a terminal, then play.  Whenever the screen changes, type what you are doing and
press Enter ("garage", "shop", "mission briefing", "combat").  The chosen functions of
every loaded unit get an execute breakpoint; when one fires, the function is counted
under the current label and its breakpoint comes off after --hits hits, so the game
slows down only while new code runs.  A new label re-arms the functions, so code that
runs on several screens gets a row per screen.  Type `?` for a status line, `q` (or
Ctrl+C) to stop.

Which functions (--set; PsyQ library code never):
  all       every function (default; every function is matched now)
  unnamed   functions with no `name` row in hints.tsv and no entry in symbols.tsv
  unseen    functions with no row in coverage.tsv yet
  open      functions not in the manifest (the old default)
Re-arming skips what is already known: a function that already has a row under the
current label (this run or an earlier one), or rows under --max-contexts labels (it
runs nearly everywhere: frame loop, shared UI), is not armed again.

Units (--unit, repeatable): `all` (default: the exe, ovl201..204 and every mission
overlay), `front` (exe + ovl201), `mission` (exe + ovl202 + the story mission overlays),
`vs` (exe + ovl203/ovl204 + their arena overlays), or unit names (exe, ovl202, fdat060).
Overlays share load addresses (ovl201..204 at 0x8004ADA0; story missions at 0x801C4B40,
mode2 arenas at 0x801C8460, mode1 versus at 0x801B80C8).  A unit is armed only while its
bytes are in RAM (a 64-byte probe window, picked where its image differs from the other
images of the same slot, is compared with RAM every --sync seconds; one read per slot);
a hit is only counted when the word at the PC is the unit's own retail word, so code
swapped in under a stale breakpoint never counts.

--max-armed N keeps at most N breakpoints in place (functions seen on the fewest screens
first) and rotates the rest in every --rotate seconds: use it if the game crawls with
a whole overlay armed.

Arguments and return values (--capture, repeatable: `ovl201:0x8009C964`, an exe
address, or a name from hints.tsv such as `ac_calc_performance`): on entry the script
reads a0..a3 (and --deref bytes behind each argument that points into RAM), sets a
breakpoint on the return address, and on return reads v0/v1 and the same bytes again.
Up to --captures calls per function per label.  One row per call goes to
re/match/calls.tsv.  Captured functions are also counted for coverage.

Output (merged into what is already there, written every few seconds and on exit):
  re/match/coverage.tsv   unit  vaddr  context  hits
  re/match/hints.tsv      one `hint` row per function hit this run:
                          "play coverage: garage (3), shop (1)" (totals from coverage.tsv);
                          a function whose summary is already the newest such row is skipped
  re/match/calls.tsv      (--capture) unit vaddr context a0..a3 v0 v1 ra in out timestamp

The MCP server keeps a single session: starting this script ends any other client's
session (another Claude session's trap script, for example).  Agree who owns port
2346 first.  On exit the script removes its own breakpoints and resumes the game if
it is paused on one of them.
"""
import argparse
import datetime
import json
import os
import queue
import re
import sys
import threading
import time
import urllib.error
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
LOOP = os.path.join(REPO, "tools", "loop")
sys.path.insert(0, LOOP)

# 127.0.0.1, not localhost: on Windows "localhost" tries ::1 first and every call then
# waits ~2 s for that to fail before falling back to IPv4 (33 ms vs 2074 ms per call here).
URL = "http://127.0.0.1:2346/mcp"
COVERAGE = os.path.join(REPO, "re", "match", "coverage.tsv")
HINTS = os.path.join(REPO, "re", "match", "hints.tsv")
CALLS = os.path.join(REPO, "re", "match", "calls.tsv")
SYMBOLS = os.path.join(REPO, "include", "ac1", "symbols.tsv")
CALL_COLS = ["unit", "vaddr", "context", "a0", "a1", "a2", "a3", "v0", "v1", "ra", "in", "out", "timestamp"]
COV_COLS = ["unit", "vaddr", "context", "hits"]
HINT_COLS = ["unit", "vaddr", "kind", "text", "timestamp"]
HINT_PREFIX = "play coverage: "
BROAD = 8                  # more labels than this: summarised as "broad"
PROBE = 64
PROGRAM_OVERLAYS = ("ovl201", "ovl202", "ovl203", "ovl204")
UNIT_PRESETS = {           # --unit presets: program overlays + the mission slots they call
    "front": ("exe", "ovl201"),
    "mission": ("exe", "ovl202", 0x801C4B40),
    "vs": ("exe", "ovl203", "ovl204", 0x801B80C8, 0x801C8460),
}
FUNCTION_SETS = ("all", "unnamed", "unseen", "open")
RAM = (0x80000000, 0x80200000)
MAX_PENDING = 64           # return breakpoints waiting at once (--capture)


class McpError(Exception):
    pass


# ---------------------------------------------------------------------------
# MCP over HTTP JSON-RPC (stdlib only)
# ---------------------------------------------------------------------------
def parse_body(content_type, text):
    """JSON-RPC message from a JSON or SSE (`data:` lines, last one wins) response body."""
    if "text/event-stream" in (content_type or ""):
        out = None
        for line in text.splitlines():
            if line.startswith("data:"):
                try:
                    out = json.loads(line[5:].strip())
                except ValueError:
                    continue
        return out
    try:
        return json.loads(text) if text.strip() else None
    except ValueError:
        return None


def unwrap(msg):
    """tools/call response -> the tool's JSON value (or text); raises McpError on errors."""
    if not isinstance(msg, dict):
        raise McpError("no JSON-RPC response")
    if "error" in msg:
        e = msg["error"]
        raise McpError(e.get("message", str(e)) if isinstance(e, dict) else str(e))
    res = msg.get("result", {})
    texts = [c.get("text", "") for c in res.get("content", []) if c.get("type") == "text"]
    vals = []
    for t in texts:
        try:
            vals.append(json.loads(t))
        except ValueError:
            vals.append(t)
    val = vals[0] if len(vals) == 1 else (vals or res)
    if res.get("isError"):
        raise McpError(val.get("message", str(val)) if isinstance(val, dict) else str(val))
    return val


class Mcp:
    def __init__(self, url=URL, timeout=30):
        self.url, self.timeout, self.sid, self.n = url, timeout, None, 0

    def _post(self, body):
        data = json.dumps(body).encode("utf-8")
        h = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
        if self.sid:
            h["Mcp-Session-Id"] = self.sid
        req = urllib.request.Request(self.url, data=data, headers=h, method="POST")
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as r:
                sid = r.headers.get("Mcp-Session-Id")
                return sid, parse_body(r.headers.get("Content-Type", ""), r.read().decode("utf-8", "replace"))
        except urllib.error.HTTPError as e:
            if e.code in (400, 404):
                raise McpError("HTTP %d: MCP session lost (another client called initialize?)" % e.code)
            raise McpError("HTTP %d from %s" % (e.code, self.url))
        except (urllib.error.URLError, OSError) as e:
            raise McpError("cannot reach %s (%s): is DuckStation running with the MCP server on?"
                           % (self.url, getattr(e, "reason", e)))

    def initialize(self):
        sid, msg = self._post({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                               "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                                          "clientInfo": {"name": "ac1-coverage-tag", "version": "1.0"}}})
        if not sid:
            raise McpError("initialize returned no Mcp-Session-Id")
        self.sid = sid
        self._post({"jsonrpc": "2.0", "method": "notifications/initialized"})
        return (msg or {}).get("result", {}).get("serverInfo", {})

    def call(self, name, args=None):
        self.n += 1
        _sid, msg = self._post({"jsonrpc": "2.0", "id": self.n + 1, "method": "tools/call",
                                "params": {"name": name, "arguments": args or {}}})
        return unwrap(msg)

    def read(self, addr, size):
        r = self.call("read_memory", {"address": "0x%08X" % addr, "size": size})
        data = r.get("data") if isinstance(r, dict) else None
        if not isinstance(data, str):
            raise McpError("read_memory 0x%08X: unexpected reply %r" % (addr, r))
        return bytes.fromhex(data.replace(" ", ""))


# ---------------------------------------------------------------------------
# images, probes, functions
# ---------------------------------------------------------------------------
def pick_probe(image, others, start=0, size=PROBE):
    """Offset of the first `size`-byte window at/after `start` (step `size`) that is not all
    zero and differs from the same window of every other image; None if there is none."""
    for off in range(start - start % size, len(image) - size + 1, size):
        win = image[off:off + size]
        if not any(win):
            continue
        if all(o[off:off + size] != win for o in others):
            return off
    return None


class Unit:
    def __init__(self, name, image, base, functions):
        self.name, self.image, self.base = name, image, base
        self.functions = functions            # [(vaddr int, size)]
        self.probe = None                     # file offset of the probe window
        self.loaded = False

    def word(self, vaddr):
        off = vaddr - self.base
        return self.image[off:off + 4]

    def probe_bytes(self):
        return self.image[self.probe:self.probe + PROBE]


def expand_units(names, all_units, bases):
    """--unit values -> unit names in order: presets, `all`, slot addresses, plain names."""
    out = []
    for n in names:
        if n == "all":
            picked = all_units
        elif n in UNIT_PRESETS:
            picked = []
            for x in UNIT_PRESETS[n]:
                picked += [u for u in all_units if bases.get(u) == x] if isinstance(x, int) else [x]
        else:
            picked = [n]
        out += [u for u in picked if u not in out]
    return out


def named_functions(hints_path=None, symbols_path=None):
    """{(unit, 0xADDR)} that already have a name: `name` rows in hints.tsv, symbols.tsv."""
    import units as U
    out = set()
    for r in read_hints(hints_path or HINTS):
        if r["kind"] == "name" and r["text"].strip():
            try:
                out.add((r["unit"], norm_vaddr(r["vaddr"])))
            except ValueError:
                continue
    path = symbols_path or SYMBOLS
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as fh:
            for line in fh:
                p = line.rstrip("\r\n").split("\t")
                if len(p) < 4 or line.startswith("#") or "(" not in p[2]:
                    continue                  # functions only: C type "ret (params)"
                try:
                    out.add((U.norm(p[3] if not p[3].isdigit() else "ovl" + p[3]), norm_vaddr(p[0])))
                except (U.UnitError, ValueError):
                    continue
    return out


def pick_functions(rows, fset, matched=frozenset(), named=frozenset(), covered=frozenset()):
    """functions.tsv rows of one --set (PsyQ rows never)."""
    out = []
    for r in rows:
        if r.get("is_psyq"):
            continue
        k = (r["unit"], norm_vaddr(r["vaddr"]))
        if fset == "open" and k in matched or fset == "unnamed" and k in named \
                or fset == "unseen" and k in covered:
            continue
        out.append(r)
    return out


def load_units(names, fset="all", covered=frozenset(), hints_path=None, log=print):
    import units as U     # tools/loop
    import rank
    import drive
    rows = rank.load_functions()
    matched = set()
    if fset == "open":
        for r in drive.read_manifest()[1]:
            try:
                matched.add((U.norm(r["unit"] or "exe"), drive.ledger.norm_vaddr(r["vaddr"])))
            except (U.UnitError, ValueError):
                continue
    named = named_functions(hints_path) if fset == "unnamed" else set()
    picked = pick_functions(rows, fset, matched, named, covered)
    resolved, bases = {}, {}
    for n in sorted({r["unit"] for r in rows}):
        try:
            resolved[n] = U.resolve(n)
        except (U.UnitError, ValueError):
            continue
        bases[n] = resolved[n].base
    images, out = {}, []
    for n in expand_units(names, sorted(resolved, key=lambda x: (x != "exe", x)), bases):
        u = resolved.get(n) or U.resolve(n)
        if u.image_path not in images:
            with open(u.image_path, "rb") as fh:
                images[u.image_path] = fh.read()
        img = images[u.image_path]
        twin = next((o.name for o in out if o.base == u.base and o.image == img), None)
        if twin:
            log("%s: same image as %s, counted as %s" % (u.name, twin, twin))
            continue
        fns = sorted((int(r["vaddr"], 16), int(r["size"])) for r in picked if r["unit"] == u.name)
        out.append(Unit(u.name, img, u.base, fns))
    # slot mates: images that load at the same address
    for unit in out:
        if unit.name == "exe":
            continue
        mates = []
        for m, mu in resolved.items():
            if m != unit.name and mu.base == unit.base:
                if mu.image_path not in images:
                    with open(mu.image_path, "rb") as fh:
                        images[mu.image_path] = fh.read()
                if images[mu.image_path] != unit.image:
                    mates.append(images[mu.image_path])
        first = (unit.functions[0][0] - unit.base) if unit.functions else 0
        unit.probe = pick_probe(unit.image, mates, max(first, 0))
        if unit.probe is None:
            unit.probe = pick_probe(unit.image, mates, 0)
        if unit.probe is None:
            raise SystemExit("coverage_tag: no window tells %s apart from its slot mates" % unit.name)
    return out


def resolve_capture(spec, rows, named_rows):
    """`ovl201:0x8009C964`, `0x80012048` (exe or a unique address) or a hints.tsv name ->
    (unit, vaddr int).  Raises ValueError with the reason."""
    fns = {(r["unit"], int(r["vaddr"], 16)) for r in rows if not r.get("is_psyq")}
    if ":" in spec:
        unit, v = spec.split(":", 1)
        k = (unit, int(v, 16))
        if k not in fns:
            raise ValueError("%s is not a function start" % spec)
        return k
    if re.fullmatch(r"(0x)?[0-9A-Fa-f]{8}", spec):
        hits = [k for k in fns if k[1] == int(spec, 16)]
        if len(hits) != 1:
            raise ValueError("%s: %s; write unit:address" % (
                spec, "no function starts there" if not hits else "in " + ", ".join(sorted(u for u, _v in hits))))
        return hits[0]
    hits = sorted({(r["unit"], int(r["vaddr"], 16)) for r in named_rows
                   if r["kind"] == "name" and r["text"].strip() == spec})
    if len(hits) != 1:
        raise ValueError("name %s: %s" % (spec, "not in hints.tsv" if not hits else
                                          "several functions: " + ", ".join("%s:0x%08X" % h for h in hits)))
    return hits[0]


# ---------------------------------------------------------------------------
# coverage.tsv / hints.tsv
# ---------------------------------------------------------------------------
def norm_vaddr(v):
    v = str(v).strip()
    return "0x%08X" % int(v, 16)


def clean(s):
    return " ".join(str(s).replace("\t", " ").split())


def read_coverage(path):
    """{(unit, 0xADDR, context): hits}."""
    out = {}
    if not os.path.isfile(path):
        return out
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            if len(p) < 4 or p[0] == "unit" or line.startswith("#"):
                continue
            try:
                k = (p[0].strip(), norm_vaddr(p[1]), clean(p[2]))
                out[k] = out.get(k, 0) + int(p[3])
            except ValueError:
                continue
    return out


def write_coverage(path, cov):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(COV_COLS) + "\n")
        for (u, v, c), n in sorted(cov.items(), key=lambda kv: (kv[0][0], int(kv[0][1], 16), kv[0][2])):
            fh.write("%s\t%s\t%s\t%d\n" % (u, v, c, n))
    os.replace(tmp, path)


def merge(base, new):
    out = dict(base)
    for k, n in new.items():
        out[k] = out.get(k, 0) + n
    return out


def summary(cov, unit, vaddr):
    """'play coverage: garage (3), shop (1)' over every context of one function, most hits first."""
    ctx = [(c, n) for (u, v, c), n in cov.items() if u == unit and v == vaddr]
    ctx.sort(key=lambda cn: (-cn[1], cn[0]))
    if len(ctx) > BROAD:
        # on most screens: a frame loop or shared UI code; the list itself says little
        return HINT_PREFIX + "broad, %d screens (e.g. %s)" % (len(ctx), ", ".join(c for c, _n in ctx[:3]))
    return HINT_PREFIX + ", ".join("%s (%d)" % cn for cn in ctx)


def read_hints(path):
    rows = []
    if not os.path.isfile(path):
        return rows
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            p = line.rstrip("\r\n").split("\t")
            if len(p) < 4 or p[:2] == ["unit", "vaddr"] or line.startswith("#"):
                continue
            rows.append(dict(zip(HINT_COLS, p + [""] * (5 - len(p)))))
    return rows


def append_hints(path, cov, functions, now=None):
    """Append one `hint` row per (unit, vaddr) in `functions` whose coverage summary differs
    from that function's newest play-coverage hint.  -> rows written."""
    now = now or datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    last = {}
    for r in read_hints(path):
        if r["kind"] == "hint" and r["text"].startswith(HINT_PREFIX):
            try:
                last[(r["unit"], norm_vaddr(r["vaddr"]))] = r["text"]
            except ValueError:
                continue
    rows = []
    for unit, vaddr in sorted(functions, key=lambda k: (k[0], int(k[1], 16))):
        text = clean(summary(cov, unit, vaddr))
        if text == HINT_PREFIX.strip() or last.get((unit, vaddr)) == text:
            continue
        rows.append([unit, vaddr, "hint", text, now])
    if rows:
        new = not os.path.isfile(path) or os.path.getsize(path) == 0
        os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
        with open(path, "a", encoding="utf-8", newline="\n") as fh:
            if new:
                fh.write("\t".join(HINT_COLS) + "\n")
            for r in rows:
                fh.write("\t".join(r) + "\n")
    return rows


# ---------------------------------------------------------------------------
# the session
# ---------------------------------------------------------------------------
def reg_int(v):
    if isinstance(v, int):
        return v & 0xFFFFFFFF
    return int(str(v), 16) & 0xFFFFFFFF


def history_of(cov):
    """{(unit, 0xADDR): {contexts}} from coverage rows."""
    out = {}
    for (u, v, c) in cov:
        out.setdefault((u, v), set()).add(c)
    return out


class Session:
    def __init__(self, mcp, units, hits_per_arm, log=print, history=None, max_contexts=0,
                 max_armed=0, captures=(), captures_per_label=5, deref=16):
        self.mcp, self.units, self.cap, self.log = mcp, units, hits_per_arm, log
        self.armed = {}          # vaddr -> [unit, hits left]   (coverage)
        self.queue = []          # [(unit, vaddr)] waiting for a free slot (--max-armed)
        self.new = {}            # (unit, vaddr, context) -> hits this run
        self.seen = set()        # (unit, vaddr) hit this run
        self.history = history if history is not None else {}   # (unit, 0xADDR) -> {contexts}
        self.max_contexts, self.max_armed = max_contexts, max_armed
        self.capt_want = {v: unit for unit, v in captures}       # vaddr -> unit name (--capture)
        self.capt = {}           # vaddr -> [unit, calls left]   (armed captures)
        self.per_label, self.deref = captures_per_label, deref
        self.returns = {}        # return address -> [pending call records]
        self.calls = []          # finished call records (rows for calls.tsv)
        self.bps = set()         # every address we have a breakpoint on
        self.context = "unlabeled"
        self.calls_t = []

    def call(self, name, args=None):
        t = time.time()
        try:
            return self.mcp.call(name, args)
        finally:
            self.calls_t.append(time.time() - t)

    def bp(self, action, vaddr):
        return self.call("breakpoint", {"action": action, "type": "execute", "address": "0x%08X" % vaddr})

    def need(self, v):
        if v not in self.bps:
            self.bp("add", v)
            self.bps.add(v)

    def drop(self, v):
        """Remove the breakpoint at v once no role (coverage, capture, return) uses it."""
        if v in self.bps and v not in self.armed and v not in self.capt and v not in self.returns:
            try:
                self.bp("remove", v)
            except McpError:
                pass
            self.bps.discard(v)

    def is_loaded(self, unit):
        if unit.name == "exe":
            return True
        try:
            return self.mcp.read(unit.base + unit.probe, PROBE) == unit.probe_bytes()
        except McpError:
            return False

    def loaded_now(self):
        """{unit name: bool}, one RAM read per load slot when its probes fit in 64 KB."""
        out, slots = {}, {}
        for u in self.units:
            if u.name == "exe" or u.probe is None:
                out[u.name] = True
            else:
                slots.setdefault(u.base, []).append(u)
        for base, us in slots.items():
            lo = min(u.probe for u in us)
            hi = max(u.probe for u in us) + PROBE
            if len(us) > 1 and hi - lo <= 0x10000:
                try:
                    ram = self.mcp.read(base + lo, hi - lo)
                except McpError:
                    ram = b""
                for u in us:
                    out[u.name] = ram[u.probe - lo:u.probe - lo + PROBE] == u.probe_bytes()
            else:
                for u in us:
                    out[u.name] = self.is_loaded(u)
        return out

    def wanted(self, unit, v):
        """Arm this function under the current label?"""
        known = self.history.get((unit.name, "0x%08X" % v), ())
        if self.context in known:
            return False
        return not (self.max_contexts and len(known) >= self.max_contexts)

    def arm(self, unit):
        n = 0
        todo = [v for v, _size in unit.functions if self.wanted(unit, v)]
        todo.sort(key=lambda v: len(self.history.get((unit.name, "0x%08X" % v), ())))
        for v in todo:
            if v in self.armed:
                self.armed[v][1] = self.cap
                continue
            if self.max_armed and len(self.armed) >= self.max_armed:
                if (unit, v) not in self.queue:
                    self.queue.append((unit, v))
                continue
            self.armed[v] = [unit, self.cap]
            self.need(v)
            n += 1
        for v, name in self.capt_want.items():
            if name == unit.name and v not in self.capt:
                self.capt[v] = [unit, self.per_label]
                self.need(v)
        return n

    def fill(self):
        """Move queued functions into free --max-armed slots."""
        n = 0
        while self.queue and (not self.max_armed or len(self.armed) < self.max_armed):
            unit, v = self.queue.pop(0)
            if not unit.loaded or v in self.armed or not self.wanted(unit, v):
                continue
            self.armed[v] = [unit, self.cap]
            self.need(v)
            n += 1
        return n

    def rotate(self):
        """Swap armed functions that have not fired since they were armed for queued ones."""
        if not self.queue:
            return 0
        idle = [v for v, (_u, left) in self.armed.items() if left == self.cap][:len(self.queue)]
        for v in idle:
            unit = self.armed.pop(v)[0]
            self.drop(v)
            self.queue.append((unit, v))
        return self.fill()

    def disarm(self, unit=None):
        for v in [v for v, (u, _n) in self.armed.items() if unit is None or u is unit]:
            del self.armed[v]
            self.drop(v)
        for v in [v for v, (u, _n) in self.capt.items() if unit is None or u is unit]:
            del self.capt[v]
            self.drop(v)
        self.queue = [(u, v) for u, v in self.queue if unit is not None and u is not unit]

    def sync(self, rearm=False):
        """Arm units whose bytes are in RAM, disarm the others; rearm=True re-arms all."""
        state = self.loaded_now()
        for u in self.units:
            now = state.get(u.name, False)
            if now and (rearm or not u.loaded):
                t = time.time()
                first = not u.loaded
                u.loaded = True
                if rearm:
                    for v in [v for v, (cu, _n) in self.capt.items() if cu is u]:
                        self.capt[v][1] = self.per_label
                n = self.arm(u)
                if n or first:
                    self.log("%s loaded: %d breakpoint(s) armed in %.1f s%s"
                             % (u.name, n, time.time() - t,
                                ", %d queued" % len(self.queue) if self.queue else ""))
            elif not now and u.loaded:
                self.disarm(u)
                self.log("%s swapped out: breakpoints removed" % u.name)
            u.loaded = now
        self.fill()

    def word_ok(self, unit, pc):
        try:
            return self.mcp.read(pc, 4) == unit.word(pc)
        except McpError:
            return False

    def count(self, unit, pc):
        k = (unit.name, "0x%08X" % pc, self.context)
        self.new[k] = self.new.get(k, 0) + 1
        self.history.setdefault(k[:2], set()).add(self.context)
        if k[:2] not in self.seen:
            self.seen.add(k[:2])
            self.log("  hit %s %s  [%s]" % (unit.name, k[1], self.context))

    def on_pause(self, pc):
        """-> True when the pause was ours (and the game should continue)."""
        ours = pc in self.bps        # a return breakpoint hit by another call frame is still ours
        if pc in self.returns:
            self.on_return(pc)
        ent = self.armed.get(pc)
        unit = (ent or self.capt.get(pc) or [None])[0]
        if unit is None:
            return ours
        if not self.word_ok(unit, pc):
            self.log("0x%08X: RAM is not %s any more; re-syncing overlays" % (pc, unit.name))
            self.disarm(unit)
            unit.loaded = False
            return True
        if pc in self.capt:
            self.on_entry(unit, pc)
        self.count(unit, pc)
        if ent:
            ent[1] -= 1
            if ent[1] <= 0:
                del self.armed[pc]
                self.drop(pc)
                self.fill()
        return True

    def regs(self):
        r = self.call("read_registers", {"group": "gpr"})
        return {k: reg_int(v) for k, v in r.items()} if isinstance(r, dict) else {}

    def peek(self, values):
        """'a0:hex;a2:hex' for every value that points into RAM (--deref bytes each)."""
        if not self.deref:
            return ""
        out = []
        for name, v in values:
            if RAM[0] <= v <= RAM[1] - self.deref:
                try:
                    out.append("%s:%s" % (name, self.mcp.read(v, self.deref).hex()))
                except McpError:
                    continue
        return ";".join(out)

    def on_entry(self, unit, pc):
        ent = self.capt[pc]
        r = self.regs()
        args = [(n, r.get(n, 0)) for n in ("a0", "a1", "a2", "a3")]
        rec = {"unit": unit.name, "vaddr": "0x%08X" % pc, "context": self.context,
               "ra": "0x%08X" % r.get("ra", 0), "sp": r.get("sp"), "args": args, "in": self.peek(args)}
        for n, v in args:
            rec[n] = "0x%08X" % v
        ra = r.get("ra", 0)
        if ra and sum(len(x) for x in self.returns.values()) < MAX_PENDING:
            self.returns.setdefault(ra, []).append(rec)
            self.need(ra)
        ent[1] -= 1
        if ent[1] <= 0:
            del self.capt[pc]
            self.drop(pc)

    def on_return(self, pc):
        r = self.regs()
        pend = self.returns.get(pc, [])
        rec = next((x for x in reversed(pend) if x["sp"] == r.get("sp")), None)
        if rec is None:
            return False
        pend.remove(rec)
        if not pend:
            del self.returns[pc]
            self.drop(pc)
        rec["v0"], rec["v1"] = "0x%08X" % r.get("v0", 0), "0x%08X" % r.get("v1", 0)
        rec["out"] = self.peek(rec["args"])
        rec["timestamp"] = datetime.datetime.now().replace(microsecond=0).isoformat()
        self.calls.append(rec)
        self.log("  call %s %s(%s) -> v0=%s  [%s]" % (rec["unit"], rec["vaddr"],
                                                      ", ".join(rec[n] for n in ("a0", "a1", "a2", "a3")),
                                                      rec["v0"], rec["context"]))
        return True


def append_calls(path, calls):
    if not calls:
        return 0
    new = not os.path.isfile(path) or os.path.getsize(path) == 0
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(CALL_COLS) + "\n")
        for c in calls:
            fh.write("\t".join(clean(c.get(k, "")) for k in CALL_COLS) + "\n")
    return len(calls)


def cleanup(mcp, sess):
    """Remove our breakpoints and resume the game if it stopped on one.  If another client
    took the MCP session, take it back once for this: leaving hundreds of breakpoints in a
    paused game is worse than ending the other client's session."""
    for attempt in (0, 1):
        try:
            st = mcp.call("wait_for_pause")
            ours = isinstance(st, dict) and st.get("status") == "paused" and \
                int(st.get("pc", "0"), 16) in sess.bps
            for v in sorted(sess.bps):
                mcp.call("breakpoint", {"action": "remove", "type": "execute", "address": "0x%08X" % v})
                sess.bps.discard(v)
            sess.armed.clear()
            sess.capt.clear()
            sess.returns.clear()
            if ours:
                mcp.call("continue")
            return True
        except McpError as e:
            if attempt:
                print("coverage_tag: could not remove %d breakpoint(s): %s" % (len(sess.bps), e),
                      file=sys.stderr)
                return False
            try:
                mcp.initialize()
                print("coverage_tag: re-opened the MCP session to remove %d breakpoint(s)"
                      % len(sess.bps), file=sys.stderr)
            except McpError:
                pass
    return False


def shown(path):
    """Repo-relative path for messages, or the path itself when it is outside the repo."""
    try:
        rel = os.path.relpath(os.path.abspath(path), REPO)
    except ValueError:            # another drive
        return path
    return path if rel.startswith("..") else rel.replace("\\", "/")


def stdin_labels(q, quit_on_eof=True):
    """Labels typed in the terminal; end of input quits unless a --minutes limit runs the show."""
    try:
        for line in sys.stdin:
            q.put(line.strip())
    except (OSError, ValueError):
        pass
    if quit_on_eof:
        q.put("q")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--unit", action="append",
                    help="all (default), front, mission, vs, or exe / ovl201..ovl204 / fdatNNN (repeatable)")
    ap.add_argument("--set", choices=FUNCTION_SETS, default=None,
                    help="which functions: all (default), unnamed, unseen (no coverage row yet), open")
    ap.add_argument("--context", default="unlabeled", help="label to start with (default: unlabeled)")
    ap.add_argument("--hits", type=int, default=1, help="hits counted per function per label before its breakpoint comes off")
    ap.add_argument("--minutes", type=float, default=None, help="stop after this many minutes")
    ap.add_argument("--sync", type=float, default=2.0, help="seconds between overlay checks (default 2)")
    ap.add_argument("--all-functions", action="store_true", help="same as --set all (kept for old commands)")
    ap.add_argument("--max-contexts", type=int, default=BROAD,
                    help="don't re-arm a function seen under this many labels already (default %d, 0 = no cap)" % BROAD)
    ap.add_argument("--max-armed", type=int, default=0,
                    help="at most this many coverage breakpoints at once, the rest rotate in (default 0 = no cap)")
    ap.add_argument("--rotate", type=float, default=10.0, help="seconds between rotations with --max-armed (default 10)")
    ap.add_argument("--capture", action="append", default=[],
                    help="record arguments and return value: unit:0xADDR, an exe address or a hints.tsv name (repeatable)")
    ap.add_argument("--captures", type=int, default=5, help="calls recorded per captured function per label (default 5)")
    ap.add_argument("--deref", type=int, default=16, help="bytes read behind each RAM-pointer argument (default 16, 0 = off)")
    ap.add_argument("--calls", default=CALLS, help="calls.tsv to append captured calls to")
    ap.add_argument("--out", default=COVERAGE)
    ap.add_argument("--hints", default=HINTS, help="hints.tsv to append to ('' = none)")
    ap.add_argument("--url", default=URL)
    ap.add_argument("--check", action="store_true", help="health check: server info, loaded overlay, call latency")
    args = ap.parse_args(argv)

    names = args.unit or ["all"]
    fset = args.set or "all"
    if args.set and args.all_functions and args.set != "all":
        ap.error("--all-functions contradicts --set %s" % args.set)
    cov0 = read_coverage(args.out)
    units = load_units(names, fset, {(u, v) for (u, v, _c) in cov0}, args.hints or None)
    captures = []
    if args.capture:
        import rank
        rows, named = rank.load_functions(), read_hints(args.hints or HINTS)
        for spec in args.capture:
            try:
                captures.append(resolve_capture(spec, rows, named))
            except ValueError as e:
                print("coverage_tag: --capture %s" % e, file=sys.stderr)
                return 3
        missing = {u for u, _v in captures} - {u.name for u in units}
        if missing:
            print("coverage_tag: --capture needs unit(s) %s in --unit" % ", ".join(sorted(missing)), file=sys.stderr)
            return 3
    mcp = Mcp(args.url)
    try:
        info = mcp.initialize()
    except McpError as e:
        print("coverage_tag: %s" % e, file=sys.stderr)
        return 2
    print("MCP: %s %s" % (info.get("name", "?"), info.get("version", "")))
    sess = Session(mcp, units, max(1, args.hits), history=history_of(cov0), max_contexts=args.max_contexts,
                   max_armed=args.max_armed, captures=captures, captures_per_label=max(1, args.captures),
                   deref=args.deref)
    total = sum(len(u.functions) for u in units)
    if len(units) > 8:
        print("%d unit(s), %d function(s) (--set %s)" % (len(units), total, fset))
    else:
        for u in units:
            print("%s: %d function(s), %s" % (u.name, len(u.functions), "always loaded" if u.probe is None
                                                else "probe window 0x%08X" % (u.base + u.probe)))
    for u, v in captures:
        print("capture %s 0x%08X" % (u, v))
    if args.check:
        t = time.time()
        st = mcp.call("wait_for_pause")
        dt = time.time() - t
        state = sess.loaded_now()
        loaded = [u.name for u in units if state.get(u.name)]
        print("status: %s   call latency %.0f ms   loaded: %s"
              % (st.get("status") if isinstance(st, dict) else st, dt * 1000, ", ".join(loaded) or "none"))
        return 0

    labels = queue.Queue()
    threading.Thread(target=stdin_labels, args=(labels, args.minutes is None), daemon=True).start()
    sess.context = clean(args.context) or "unlabeled"
    print("context: %s   (type a label + Enter when the screen changes, ? = status, q = quit)" % sess.context)
    t0 = last_sync = last_save = last_rotate = time.time()
    saved_calls = 0
    status = 0
    try:
        sess.sync()
        while True:
            if args.minutes is not None and time.time() - t0 > args.minutes * 60:
                print("time limit reached")
                break
            try:
                lab = labels.get_nowait()
            except queue.Empty:
                lab = None
            if lab is not None:
                if lab.lower() in ("q", "quit", "exit"):
                    break
                if lab == "?":
                    print("context %s | %d armed, %d queued | %d function(s) hit this run | %d call(s) captured"
                          " | loaded: %s | %.0f ms/call"
                          % (sess.context, len(sess.armed), len(sess.queue), len(sess.seen), len(sess.calls),
                             ", ".join(u.name for u in units if u.loaded) or "none",
                             1000 * sum(sess.calls_t[-50:]) / max(1, len(sess.calls_t[-50:]))))
                elif lab:
                    sess.context = clean(lab)
                    print("context: %s (re-arming)" % sess.context)
                    sess.sync(rearm=True)
                    last_sync = time.time()
                continue
            st = sess.call("wait_for_pause")
            if isinstance(st, dict) and st.get("status") == "paused" and st.get("pc"):
                if sess.on_pause(int(st["pc"], 16)):
                    sess.call("continue")
                    continue
                time.sleep(0.25)          # paused by the user (or someone else): leave it
            else:
                time.sleep(0.05)
            if time.time() - last_sync > args.sync:
                sess.sync()
                last_sync = time.time()
            if args.max_armed and time.time() - last_rotate > args.rotate:
                sess.rotate()
                last_rotate = time.time()
            if time.time() - last_save > 10 and sess.new:
                write_coverage(args.out, merge(cov0, sess.new))
                saved_calls += append_calls(args.calls, sess.calls[saved_calls:])
                last_save = time.time()
    except KeyboardInterrupt:
        print("stopping")
    except McpError as e:
        print("coverage_tag: %s" % e, file=sys.stderr)
        status = 2
    finally:
        cleanup(mcp, sess)
    cov = merge(cov0, sess.new)
    write_coverage(args.out, cov)
    rows = append_hints(args.hints, cov, sess.seen) if args.hints else []
    saved_calls += append_calls(args.calls, sess.calls[saved_calls:])
    ctxs = sorted({c for (_u, _v, c) in sess.new})
    print("%d function(s) hit in %d context(s) [%s]; %s updated, %d hint row(s) added%s"
          % (len(sess.seen), len(ctxs), ", ".join(ctxs), shown(args.out),
             len(rows), "; %d call(s) appended to %s" % (saved_calls, shown(args.calls))
             if saved_calls else ""))
    return status


if __name__ == "__main__":
    sys.exit(main())
