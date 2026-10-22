#!/usr/bin/env python3
"""
browser-tools/launchpad/server.py — one page that lists, starts and opens every browser tool.

    python browser-tools/launchpad/server.py --open      # http://127.0.0.1:8760/
    (or double-click browser-tools/launch.cmd, or the "AC1 Browser Tools" desktop shortcut)

Reads browser-tools/*/tool.json (see browser-tools/README.md). For each tool it shows
whether something answers on its port, and can start it (with a chosen variant) as a
background process whose output goes to a log in the temp dir. It stops only processes it
started itself. 127.0.0.1 only; stdlib only.

    GET  /                  the page
    GET  /api/tools         manifests + running state
    POST /api/start         {"id": tool, "variant": i}   -> {"ok", "url"} once the port answers
    POST /api/stop          {"id": tool}
    GET  /api/log?id=TOOL   the last 4 KB of that tool's log
    GET  /readme?id=TOOL    the tool's README as plain text
"""
from __future__ import annotations
import argparse, os, subprocess, sys, tempfile, time
from urllib.parse import urlparse, parse_qs

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(HERE), "_shared"))
import btserve  # noqa: E402

PORT = 8760
LOGS = os.path.join(tempfile.gettempdir(), "ac1-browser-tools")
PROCS = {}                       # tool id -> Popen (started here)


def tools():
    out = []
    for m in btserve.manifests():
        p = PROCS.get(m["id"])
        alive = p is not None and p.poll() is None
        out.append(dict(
            id=m["id"], name=m.get("name", m["id"]), summary=m.get("summary", ""),
            port=m["port"], url=btserve.url_for(m["port"]), needs=m.get("needs", []),
            variants=[v.get("label", "") for v in m.get("variants", [{"label": "default"}])],
            launcher=m.get("launcher"), running=btserve.is_running(m["port"]),
            started_here=alive, exited=(p.poll() if p is not None and not alive else None)))
    return out


def manifest(tid):
    for m in btserve.manifests():
        if m["id"] == tid:
            return m
    raise KeyError(tid)


def start(tid, variant=0):
    m = manifest(tid)
    url = btserve.url_for(m["port"])
    if btserve.is_running(m["port"]):
        return dict(ok=True, url=url, note="already running")
    variants = m.get("variants") or [{"args": []}]
    args = [a.replace("{port}", str(m["port"])) for a in m["start"]]
    args[0] = os.path.join(m["dir"], args[0])
    cmd = [btserve.python()] + args + list(variants[max(0, min(variant, len(variants) - 1))].get("args", []))
    os.makedirs(LOGS, exist_ok=True)
    log = open(os.path.join(LOGS, tid + ".log"), "w", encoding="utf-8")
    log.write("$ " + " ".join(cmd) + "\n"); log.flush()
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    env = dict(os.environ, PYTHONIOENCODING="utf-8", PYTHONUNBUFFERED="1")
    PROCS[tid] = p = subprocess.Popen(cmd, cwd=btserve.REPO, stdout=log, stderr=subprocess.STDOUT,
                                      stdin=subprocess.DEVNULL, creationflags=flags, env=env)
    t0 = time.time()
    while time.time() - t0 < 20:
        if btserve.is_running(m["port"]):
            return dict(ok=True, url=url)
        if p.poll() is not None:
            return dict(ok=False, error=f"exited with code {p.returncode}", log=log_tail(tid))
        time.sleep(0.2)
    return dict(ok=False, error="no answer on port %d after 20 s (still starting?)" % m["port"], log=log_tail(tid))


def stop(tid):
    p = PROCS.get(tid)
    if p is None or p.poll() is not None:
        return dict(ok=False, error="not started from the launchpad; stop it where it runs")
    p.terminate()
    try:
        p.wait(5)
    except subprocess.TimeoutExpired:
        p.kill()
    return dict(ok=True)


def log_tail(tid, n=4096):
    path = os.path.join(LOGS, tid + ".log")
    if not os.path.exists(path):
        return ""
    with open(path, "rb") as fh:
        fh.seek(max(0, os.path.getsize(path) - n))
        return fh.read().decode("utf-8", "replace")


class Handler(btserve.Handler):
    def do_GET(self):
        u = urlparse(self.path)
        q = {k: v[0] for k, v in parse_qs(u.query).items()}
        try:
            if u.path in ("/", "/index.html"):
                return self.file(os.path.join(HERE, "index.html"))
            if u.path == "/api/tools":
                return self.json(tools())
            if u.path == "/api/log":
                return self.json(dict(log=log_tail(q["id"])))
            if u.path == "/readme":
                m = manifest(q["id"])
                path = os.path.normpath(os.path.join(m["dir"], m.get("readme", "README.md")))
                return self.file(path, "text/plain")
            return self.json({"error": "not found"}, 404)
        except (KeyError, OSError) as e:
            return self.json({"error": f"{type(e).__name__}: {e}"}, 400)

    def do_POST(self):
        u = urlparse(self.path)
        try:
            b = self.body_json()
            if u.path == "/api/start":
                return self.json(start(b["id"], int(b.get("variant", 0))))
            if u.path == "/api/stop":
                return self.json(stop(b["id"]))
            return self.json({"error": "not found"}, 404)
        except (KeyError, ValueError, OSError) as e:
            return self.json({"ok": False, "error": f"{type(e).__name__}: {e}"}, 400)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", type=int, default=PORT)
    ap.add_argument("--open", action="store_true", help="open the page in the default browser")
    a = ap.parse_args()
    try:
        btserve.run(Handler, a.port, name="AC1 browser tools", open_browser=a.open)
    finally:
        for p in PROCS.values():             # tools started here go down with the launchpad
            if p.poll() is None:
                p.terminate()


if __name__ == "__main__":
    main()
