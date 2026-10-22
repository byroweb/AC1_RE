#!/usr/bin/env python3
"""coverage_tag.py - tag open functions with the game screen they run in, while you play.

    coverage_tag.py [--unit ovl201]... [--context garage] [--hits 1] [--minutes N]
                    [--all-functions] [--out re/match/coverage.tsv] [--hints re/match/hints.tsv]
    coverage_tag.py --check            MCP health check + which overlay is loaded, then exit

Start DuckStation (the MCP fork, port 2346) with the game running, start this script
in a terminal, then play.  Whenever the screen changes, type what you are doing and
press Enter ("garage", "shop", "mission briefing", "combat").  Every open function
(functions.tsv, not PsyQ, not in the manifest) of the chosen units gets an execute
breakpoint; when one fires, the function is counted under the current label and its
breakpoint comes off after --hits hits, so the game slows down only while new code
runs.  A new label re-arms every function, so code that runs on several screens gets
a row per screen.  Type `?` for a status line, `q` (or Ctrl+C) to stop.

Overlays: ovl201..204 share one load address (0x8004ADA0), mission overlays share
theirs.  A unit is armed only while its bytes are in RAM (a 64-byte probe window,
picked where its image differs from the other images of the same slot, is compared
with RAM every --sync seconds); a hit is only counted when the word at the PC is the
unit's own retail word, so code swapped in under a stale breakpoint never counts.

Output (merged into what is already there, written every few seconds and on exit):
  re/match/coverage.tsv   unit  vaddr  context  hits
  re/match/hints.tsv      one `hint` row per function hit this run:
                          "play coverage: garage (3), shop (1)" (totals from coverage.tsv);
                          a function whose summary is already the newest such row is skipped

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
COV_COLS = ["unit", "vaddr", "context", "hits"]
HINT_COLS = ["unit", "vaddr", "kind", "text", "timestamp"]
HINT_PREFIX = "play coverage: "
PROBE = 64
PROGRAM_OVERLAYS = ("ovl201", "ovl202", "ovl203", "ovl204")


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


def load_units(names, all_functions=False):
    import units as U     # tools/loop
    import rank
    import drive
    rows = rank.load_functions()
    matched = set()
    for r in drive.read_manifest()[1]:
        try:
            matched.add((U.norm(r["unit"] or "exe"), drive.ledger.norm_vaddr(r["vaddr"])))
        except (U.UnitError, ValueError):
            continue
    picked = rows if all_functions else rank.candidates(rows, matched)
    out = []
    for n in names:
        u = U.resolve(n)
        with open(u.image_path, "rb") as fh:
            img = fh.read()
        fns = sorted((int(r["vaddr"], 16), int(r["size"])) for r in picked
                     if r["unit"] == u.name and not r.get("is_psyq"))
        out.append(Unit(u.name, img, u.base, fns))
    # slot mates: images that load at the same address
    for unit in out:
        if unit.name == "exe":
            continue
        mates = []
        if unit.name in PROGRAM_OVERLAYS:
            for m in PROGRAM_OVERLAYS:
                if m != unit.name:
                    with open(U.resolve(m).image_path, "rb") as fh:
                        mates.append(fh.read())
        else:
            for r in {r["unit"] for r in rows if r["unit"].startswith("fdat")} - {unit.name}:
                try:
                    mu = U.resolve(r)
                except (U.UnitError, ValueError):
                    continue
                if mu.base == unit.base:
                    with open(mu.image_path, "rb") as fh:
                        mates.append(fh.read())
        first = (unit.functions[0][0] - unit.base) if unit.functions else 0
        unit.probe = pick_probe(unit.image, mates, max(first, 0))
        if unit.probe is None:
            unit.probe = pick_probe(unit.image, mates, 0)
        if unit.probe is None:
            raise SystemExit("coverage_tag: no window tells %s apart from its slot mates" % unit.name)
    return out


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
class Session:
    def __init__(self, mcp, units, hits_per_arm, log=print):
        self.mcp, self.units, self.cap, self.log = mcp, units, hits_per_arm, log
        self.armed = {}          # vaddr -> [unit, hits left]
        self.new = {}            # (unit, vaddr, context) -> hits this run
        self.seen = set()        # (unit, vaddr) hit this run
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

    def is_loaded(self, unit):
        if unit.name == "exe":
            return True
        try:
            return self.mcp.read(unit.base + unit.probe, PROBE) == unit.probe_bytes()
        except McpError:
            return False

    def arm(self, unit):
        n = 0
        for v, _size in unit.functions:
            if v not in self.armed:
                self.bp("add", v)
                n += 1
            self.armed[v] = [unit, self.cap]
        return n

    def disarm(self, unit=None):
        for v in [v for v, (u, _n) in self.armed.items() if unit is None or u is unit]:
            try:
                self.bp("remove", v)
            except McpError:
                pass
            del self.armed[v]

    def sync(self, rearm=False):
        """Arm units whose bytes are in RAM, disarm the others; rearm=True re-arms all."""
        for u in self.units:
            now = self.is_loaded(u)
            if now and (rearm or not u.loaded):
                t = time.time()
                n = self.arm(u)
                if n or not u.loaded:
                    self.log("%s loaded: %d breakpoint(s) armed in %.1f s" % (u.name, n, time.time() - t))
            elif not now and u.loaded:
                self.disarm(u)
                self.log("%s swapped out: breakpoints removed" % u.name)
            u.loaded = now

    def on_pause(self, pc):
        """-> True when the pause was ours (and the game should continue)."""
        ent = self.armed.get(pc)
        if not ent:
            return False
        unit = ent[0]
        try:
            ok = self.mcp.read(pc, 4) == unit.word(pc)
        except McpError:
            ok = False
        if not ok:
            self.log("0x%08X: RAM is not %s any more; re-syncing overlays" % (pc, unit.name))
            self.disarm(unit)
            unit.loaded = False
            return True
        k = (unit.name, "0x%08X" % pc, self.context)
        self.new[k] = self.new.get(k, 0) + 1
        if (unit.name, k[1]) not in self.seen:
            self.seen.add((unit.name, k[1]))
            self.log("  hit %s %s  [%s]" % (unit.name, k[1], self.context))
        ent[1] -= 1
        if ent[1] <= 0:
            self.bp("remove", pc)
            del self.armed[pc]
        return True


def cleanup(mcp, sess):
    """Remove our breakpoints and resume the game if it stopped on one.  If another client
    took the MCP session, take it back once for this: leaving hundreds of breakpoints in a
    paused game is worse than ending the other client's session."""
    for attempt in (0, 1):
        try:
            st = mcp.call("wait_for_pause")
            ours = isinstance(st, dict) and st.get("status") == "paused" and \
                int(st.get("pc", "0"), 16) in sess.armed
            for v in list(sess.armed):
                mcp.call("breakpoint", {"action": "remove", "type": "execute", "address": "0x%08X" % v})
                del sess.armed[v]
            if ours:
                mcp.call("continue")
            return True
        except McpError as e:
            if attempt:
                print("coverage_tag: could not remove %d breakpoint(s): %s" % (len(sess.armed), e),
                      file=sys.stderr)
                return False
            try:
                mcp.initialize()
                print("coverage_tag: re-opened the MCP session to remove %d breakpoint(s)"
                      % len(sess.armed), file=sys.stderr)
            except McpError:
                pass
    return False


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
    ap.add_argument("--unit", action="append", help="exe, ovl201..ovl204, fdatNNN (repeatable; default ovl201)")
    ap.add_argument("--context", default="unlabeled", help="label to start with (default: unlabeled)")
    ap.add_argument("--hits", type=int, default=1, help="hits counted per function per label before its breakpoint comes off")
    ap.add_argument("--minutes", type=float, default=None, help="stop after this many minutes")
    ap.add_argument("--sync", type=float, default=2.0, help="seconds between overlay checks (default 2)")
    ap.add_argument("--all-functions", action="store_true", help="matched functions too (default: open only)")
    ap.add_argument("--out", default=COVERAGE)
    ap.add_argument("--hints", default=HINTS, help="hints.tsv to append to ('' = none)")
    ap.add_argument("--url", default=URL)
    ap.add_argument("--check", action="store_true", help="health check: server info, loaded overlay, call latency")
    args = ap.parse_args(argv)

    names = args.unit or ["ovl201"]
    units = load_units(names, args.all_functions)
    mcp = Mcp(args.url)
    try:
        info = mcp.initialize()
    except McpError as e:
        print("coverage_tag: %s" % e, file=sys.stderr)
        return 2
    print("MCP: %s %s" % (info.get("name", "?"), info.get("version", "")))
    sess = Session(mcp, units, max(1, args.hits))
    for u in units:
        print("%s: %d function(s), %s" % (u.name, len(u.functions), "always loaded" if u.probe is None
                                            else "probe window 0x%08X" % (u.base + u.probe)))
    if args.check:
        t = time.time()
        st = mcp.call("wait_for_pause")
        dt = time.time() - t
        loaded = [u.name for u in units if sess.is_loaded(u)]
        print("status: %s   call latency %.0f ms   loaded: %s"
              % (st.get("status") if isinstance(st, dict) else st, dt * 1000, ", ".join(loaded) or "none"))
        return 0

    labels = queue.Queue()
    threading.Thread(target=stdin_labels, args=(labels, args.minutes is None), daemon=True).start()
    sess.context = clean(args.context) or "unlabeled"
    print("context: %s   (type a label + Enter when the screen changes, ? = status, q = quit)" % sess.context)
    cov0 = read_coverage(args.out)
    t0 = last_sync = last_save = time.time()
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
                    print("context %s | %d armed | %d function(s) hit this run | %.0f ms/call"
                          % (sess.context, len(sess.armed), len(sess.seen),
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
            if time.time() - last_save > 10 and sess.new:
                write_coverage(args.out, merge(cov0, sess.new))
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
    ctxs = sorted({c for (_u, _v, c) in sess.new})
    print("%d function(s) hit in %d context(s) [%s]; %s updated, %d hint row(s) added"
          % (len(sess.seen), len(ctxs), ", ".join(ctxs), os.path.relpath(args.out, REPO).replace("\\", "/"),
             len(rows)))
    return status


if __name__ == "__main__":
    sys.exit(main())
