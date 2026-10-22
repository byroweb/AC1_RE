"""
browser-tools/_shared/btserve.py — the small server kit every browser tool can use.

    import os, sys
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "_shared"))
    import btserve

    class Handler(btserve.Handler):
        def do_GET(self):
            if self.path == "/":
                return self.file(os.path.join(HERE, "index.html"))
            return self.json({"hello": "world"})

    btserve.run(Handler, port=8780, name="My tool", open_browser=args.open)

What it standardizes (browser-tools/README.md has the full checklist):
  * 127.0.0.1 only: tools serve game data and local files, never to the network;
  * --open opens the page, and a second launch while the server runs just opens the page
    (so a double-click launcher can always be clicked);
  * quiet request logging, no-store caching, JSON / bytes / file helpers;
  * the tool manifests (browser-tools/*/tool.json) and a port probe, for the launchpad.
Stdlib only.
"""
from __future__ import annotations
import json, mimetypes, os, socket, sys, threading, webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))   # browser-tools/
REPO = os.path.dirname(TOOLS)
HOST = "127.0.0.1"


class Handler(BaseHTTPRequestHandler):
    """BaseHTTPRequestHandler with the helpers every tool ends up writing."""

    def log_message(self, fmt, *args):          # quiet; tools print their own status
        pass

    def send(self, code, body, ctype):
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def json(self, obj, code=200):
        self.send(code, json.dumps(obj, separators=(",", ":")), "application/json")

    def file(self, path, ctype=None):
        with open(path, "rb") as fh:
            body = fh.read()
        ctype = ctype or mimetypes.guess_type(path)[0] or "application/octet-stream"
        if ctype.startswith("text/") and "charset" not in ctype:
            ctype += "; charset=utf-8"
        self.send(200, body, ctype)

    def body_json(self):
        n = int(self.headers.get("Content-Length") or 0)
        return json.loads(self.rfile.read(n) or b"{}")


def url_for(port):
    return f"http://{HOST}:{port}/"


def is_running(port, timeout=0.3):
    """True if something accepts connections on 127.0.0.1:port."""
    try:
        with socket.create_connection((HOST, port), timeout=timeout):
            return True
    except OSError:
        return False


class Server(ThreadingHTTPServer):
    # http.server sets SO_REUSEADDR, which on Windows lets a second server bind a port that
    # is already serving, so "already running" would never be detected. Take the port
    # exclusively there instead.
    allow_reuse_address = os.name != "nt"
    daemon_threads = True

    def server_bind(self):
        if os.name == "nt" and hasattr(socket, "SO_EXCLUSIVEADDRUSE"):
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_EXCLUSIVEADDRUSE, 1)
        super().server_bind()


def run(handler, port, name="browser tool", open_browser=False):
    """Serve `handler` on 127.0.0.1:port until Ctrl+C. If the port is taken (the tool is
    already running), open its page when asked to and return instead of failing."""
    url = url_for(port)
    try:
        if is_running(port):
            raise OSError(f"port {port} is already serving")
        srv = Server((HOST, port), handler)
    except OSError:
        if open_browser:
            print(f"{name} already running: {url}")
            webbrowser.open(url)
            return
        raise
    print(f"{name}: {url}  (Ctrl+C to stop)", flush=True)
    if open_browser:
        threading.Timer(0.5, webbrowser.open, (url,)).start()
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        srv.server_close()


def manifests():
    """Every browser-tools/<dir>/tool.json (dirs starting with _ or . skipped), sorted by
    name, each with "id" (the dir name) and "dir" added."""
    out = []
    for d in sorted(os.listdir(TOOLS)):
        p = os.path.join(TOOLS, d, "tool.json")
        if d.startswith(("_", ".")) or not os.path.isfile(p):
            continue
        with open(p, encoding="utf-8") as fh:
            m = json.load(fh)
        m["id"], m["dir"] = d, os.path.join(TOOLS, d)
        out.append(m)
    return sorted(out, key=lambda m: m.get("order", 100))


def python():
    """The interpreter to start tools with (the one running this)."""
    return sys.executable or "python"
