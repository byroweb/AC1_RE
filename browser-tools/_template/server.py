#!/usr/bin/env python3
"""
browser-tools/<name>/server.py — <one line: what this tool lets you explore>.

    python browser-tools/<name>/server.py --open        # http://127.0.0.1:<port>/

Copy browser-tools/_template/ to browser-tools/<name>/, pick a free port from the
registry in browser-tools/README.md, fill in tool.json, and replace /api/data below.
"""
from __future__ import annotations
import argparse, os, sys
from urllib.parse import urlparse, parse_qs

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(HERE), "_shared"))
import btserve  # noqa: E402

PORT = 8790          # change: one port per tool (browser-tools/README.md)


def data(q):
    """What the page plots; replace with the real thing (read from btserve.REPO paths)."""
    n = int(q.get("n", 16))
    return {"points": [[i, (i * i) % 23] for i in range(n)]}


class Handler(btserve.Handler):
    def do_GET(self):
        u = urlparse(self.path)
        q = {k: v[0] for k, v in parse_qs(u.query).items()}
        try:
            if u.path in ("/", "/index.html"):
                return self.file(os.path.join(HERE, "index.html"))
            if u.path == "/api/data":
                return self.json(data(q))
            return self.json({"error": "not found"}, 404)
        except (KeyError, ValueError) as e:
            return self.json({"error": f"{type(e).__name__}: {e}"}, 400)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", type=int, default=PORT)
    ap.add_argument("--open", action="store_true", help="open the page in the default browser")
    a = ap.parse_args()
    btserve.run(Handler, a.port, name="<tool name>", open_browser=a.open)


if __name__ == "__main__":
    main()
