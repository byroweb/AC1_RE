#!/usr/bin/env python3
"""
browser-tools/sceneview/server.py — local server for the AC1 scene viewer (index.html).

    python browser-tools/sceneview/server.py [--port 8771] [--bin DISC.bin]
    then open http://127.0.0.1:8771/

Serves, on 127.0.0.1 only (the geometry and texels are game data from your disc and
your save states; nothing is written to disk):
    /                       index.html
    /api/missions           [{n, sections}] missions whose stream has a level
    /api/level?m=N          the level of mission N (scene.level_scene) as JSON
    /api/states?m=N         [{id, mission}] library save states (all if m is omitted)
    /api/state?id=ID        player slot 0 of a state (pos, yaw, extents)
    /api/vram?id=ID         that state's 1 MB VRAM (raw 1024x512 16bpp, little endian)
    /api/vram?disc=N        mission N's level textures from the disc (RTIM.T 100 + N)

Disc image: --bin, else $AC1_BIN.  Save states: $AC1_STATES_DIR or ~/dev/ac1_states
(tools/duckstation/states.py).  Stdlib only, plus what tools/duckstation needs to
decompress a state (the zstandard package or the zstd CLI).
"""
from __future__ import annotations
import argparse, json, os, sys, threading
from urllib.parse import urlparse, parse_qs

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), "_shared"))
import btserve  # noqa: E402
import scene  # noqa: E402

_lock = threading.Lock()
_cache = {}


def cached(key, fn):
    with _lock:
        if key not in _cache:
            _cache[key] = fn()
        return _cache[key]


def missions(bin_path):
    import mission_geom as MG
    from core.fdat import fdat_entries
    ents = fdat_entries(bin_path)
    out = []
    for n in range(len(ents) // 2):
        st = ents[2 * n + 1]
        try:
            k = len(MG.placements(st, len(MG.geom_blocks(st))))
        except Exception:
            k = 0
        if k:
            out.append(dict(n=n, sections=k))
    return out


def state_by_id(sid):
    for s in scene.list_states():
        if s["id"] == sid:
            return s
    raise KeyError(sid)


class Handler(btserve.Handler):
    bin_path = None

    def do_GET(self):
        u = urlparse(self.path)
        q = {k: v[0] for k, v in parse_qs(u.query).items()}
        b = self.bin_path
        try:
            if u.path in ("/", "/index.html"):
                return self.file(os.path.join(HERE, "index.html"))
            if u.path == "/api/missions":
                return self.json(cached("missions", lambda: missions(b)))
            if u.path == "/api/level":
                n = int(q["m"])
                body = cached(("level", n), lambda: json.dumps(
                    scene.level_scene(scene.mission_stream(b, n)), separators=(",", ":")).encode())
                return self.send(200, body, "application/json")
            if u.path == "/api/states":
                m = int(q["m"]) if "m" in q else None
                return self.json([dict(id=s["id"], mission=s["mission"]) for s in scene.list_states()
                                  if m is None or s["mission"] == m])
            if u.path == "/api/vram" and "disc" in q:
                n = int(q["disc"])
                ents = cached("rtim", scene.rtim_entries)
                return self.send(200, cached(("rtim", n), lambda: scene.vram_from_rtim(n, ents)),
                                 "application/octet-stream")
            if u.path in ("/api/state", "/api/vram"):
                sid = q["id"]
                vram, player = cached(("state", sid),
                                      lambda: scene.state_info(state_by_id(sid)["file"], b))
                if u.path == "/api/state":
                    return self.json(player)
                return self.send(200, vram, "application/octet-stream")
            return self.json({"error": "not found"}, 404)
        except (KeyError, ValueError) as e:
            return self.json({"error": f"{type(e).__name__}: {e}"}, 400)
        except Exception as e:                    # keep the page alive; report the cause
            return self.json({"error": f"{type(e).__name__}: {e}"}, 500)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--port", type=int, default=8771)
    ap.add_argument("--open", action="store_true", help="open the page in the default browser")
    ap.add_argument("--bin", default=os.environ.get("AC1_BIN"),
                    help="SLUS-01323 v1.1 disc image (default $AC1_BIN)")
    a = ap.parse_args()
    if not a.bin or not os.path.exists(a.bin):
        sys.exit("disc image not found: pass --bin or set AC1_BIN")
    Handler.bin_path = a.bin
    os.environ["AC1_BIN"] = a.bin              # pa_obj.load_container reads the disc from it
    btserve.run(Handler, a.port, name="AC1 scene viewer", open_browser=a.open)


if __name__ == "__main__":
    main()
