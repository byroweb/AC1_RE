#!/usr/bin/env python3
"""
btserve_test.py — checks for browser-tools/: every tool.json is complete and consistent,
and the shared server helper serves, reports running, and refuses a taken port.
No game data needed. Exit code is nonzero on any failure.
"""
import json, os, socket, sys, threading, time, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import btserve  # noqa: E402

FAILS = []


def check(cond, msg):
    print(("ok   " if cond else "FAIL ") + msg)
    if not cond:
        FAILS.append(msg)


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def main() -> int:
    ms = btserve.manifests()
    ids = [m["id"] for m in ms]
    check({"plantview", "match-viewer", "sceneview"} <= set(ids), f"the three tools have manifests ({ids})")
    check("_template" not in ids and "launchpad" not in ids, "_template and the launchpad are not listed as tools")
    ports = [m["port"] for m in ms] + [8760]
    check(len(ports) == len(set(ports)), f"ports are unique {sorted(ports)}")
    for m in ms:
        miss = [k for k in ("name", "summary", "port", "start", "needs") if k not in m]
        check(not miss, f"{m['id']}: required keys present {miss or ''}")
        check(os.path.isfile(os.path.join(m["dir"], m["start"][0])), f"{m['id']}: start script exists")
        if m.get("launcher"):
            check(os.path.isfile(os.path.join(m["dir"], m["launcher"])), f"{m['id']}: launcher exists")
        check(os.path.isfile(os.path.normpath(os.path.join(m["dir"], m.get("readme", "README.md")))),
              f"{m['id']}: README exists")
        check("{port}" in " ".join(m["start"]), f"{m['id']}: start passes {{port}}")
    tpl = json.load(open(os.path.join(btserve.TOOLS, "_template", "tool.json"), encoding="utf-8"))
    check(tpl["port"] not in ports, "the template's port is not taken by a real tool")

    class H(btserve.Handler):
        def do_GET(self):
            return self.json({"ok": True, "path": self.path})

    port = free_port()
    t = threading.Thread(target=btserve.run, args=(H, port, "test"), daemon=True)
    t.start()
    for _ in range(50):
        if btserve.is_running(port):
            break
        time.sleep(0.05)
    check(btserve.is_running(port), "run() serves on 127.0.0.1")
    with urllib.request.urlopen(btserve.url_for(port) + "x") as r:
        body = json.load(r)
        check(body == {"ok": True, "path": "/x"} and r.headers["Cache-Control"] == "no-store",
              "Handler.json answers with no-store")
    try:
        btserve.run(H, port, "second")
        check(False, "a second run() on a taken port without --open raises")
    except OSError:
        check(True, "a second run() on a taken port without --open raises")
    check(not btserve.is_running(free_port()), "is_running is False on a free port")

    print(f"\n{len(FAILS)} failure(s)")
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
