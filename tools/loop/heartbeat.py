"""heartbeat.py - <work>/current.json: what sweep.py / permute.py are doing right now.

tools/loop/viewer.py polls it; nothing else reads it.  One JSON object, rewritten whole
(at most every --interval s, and on close) through a temp file + os.replace:

  tool       sweep | permute
  label      free text (the variants, the budget)
  pid, started, t      process id, start and last write (epoch s)
  total, finished      bodies queued, bodies done
  matches, compiles    clean matches so far, compiles so far (oracle runs, lever
                       evaluations, permuter iterations)
  active     {"unit vaddr": {phase, since, ...}} bodies being worked on now; phase is a
             sweep variant, or levers / permuter / oracle
  done       {"unit vaddr": {status, words, ...}} every finished body
  closed     true once the run ended
  paused     true while a PAUSE file holds the run (below)

A crashed run leaves closed false and a stale t; the viewer shows that as stale.

Pause: while <workdir>/PAUSE or <workdir>/../PAUSE exists (the viewer's Pause button
writes the overnight work dir's), wait_if_paused() holds each job before it starts a
body; bodies already running finish.  Delete the file to resume.
"""
import json
import os
import threading
import time


class Heartbeat:
    def __init__(self, workdir, tool, total, label="", interval=0.5, path=None):
        self.path = path or os.path.join(workdir, "current.json")
        self.interval = interval
        self.lock = threading.Lock()
        self.last = 0.0
        self.state = {"tool": tool, "label": label, "pid": os.getpid(), "started": time.time(),
                      "t": time.time(), "total": total, "finished": 0, "matches": 0, "compiles": 0,
                      "active": {}, "done": {}, "closed": False}
        self.write(force=True)

    @staticmethod
    def key(unit, vaddr):
        return "%s %s" % (unit, vaddr)

    def begin(self, unit, vaddr, phase, **kw):
        with self.lock:
            self.state["active"][self.key(unit, vaddr)] = dict(kw, phase=phase, since=time.time())
        self.write()

    def update(self, unit, vaddr, **kw):
        with self.lock:
            a = self.state["active"].get(self.key(unit, vaddr))
            if a is not None:
                if "phase" in kw and kw["phase"] != a.get("phase"):
                    a["phase_since"] = time.time()
                a.update(kw)
        self.write()

    def add(self, compiles=0, matches=0):
        with self.lock:
            self.state["compiles"] += compiles
            self.state["matches"] += matches
        self.write()

    def end(self, unit, vaddr, status, **kw):
        k = self.key(unit, vaddr)
        with self.lock:
            self.state["active"].pop(k, None)
            self.state["done"][k] = dict(kw, status=status, t=time.time())
            self.state["finished"] += 1
            if status == "MATCH":
                self.state["matches"] += 1
        self.write()

    def paused(self, on):
        with self.lock:
            self.state["paused"] = bool(on)
        self.write(force=True)

    def close(self):
        with self.lock:
            self.state["closed"] = True
            self.state["active"] = {}
        self.write(force=True)

    def write(self, force=False):
        with self.lock:
            t = time.time()
            if not force and t - self.last < self.interval:
                return
            self.last = t
            self.state["t"] = t
            text = json.dumps(self.state, separators=(",", ":"))
        tmp = "%s.%d.tmp" % (self.path, os.getpid())
        try:
            with open(tmp, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(text)
            for _ in range(5):
                try:
                    os.replace(tmp, self.path)
                    return
                except PermissionError:      # Windows: the viewer has it open this instant
                    time.sleep(0.02)
            os.unlink(tmp)
        except OSError:
            pass                             # a heartbeat must never stop the run


def pause_files(workdir):
    return [os.path.join(workdir, "PAUSE"), os.path.join(os.path.dirname(os.path.abspath(workdir)), "PAUSE")]


def wait_if_paused(workdir, hb=None, poll=1.0):
    """Block while a PAUSE file exists; tell the heartbeat."""
    if not workdir:
        return
    files = pause_files(workdir)
    if not any(os.path.exists(f) for f in files):
        return
    if hb is not None:
        hb.paused(True)
    while any(os.path.exists(f) for f in files):
        time.sleep(poll)
    if hb is not None:
        hb.paused(False)


class Null:
    """No-op stand-in when there is no work dir to write to."""
    def __getattr__(self, _name):
        return lambda *a, **k: None
