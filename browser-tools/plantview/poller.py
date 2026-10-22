#!/usr/bin/env python3
"""
browser-tools/plantview/poller.py — sample the AC1 mission-loop state and serve it to
browser-tools/plantview/index.html as JSON (HTTP + server-sent events, localhost only).

Sources
-------
  --source live        read RAM from the running DuckStation (sadnescity MCP fork,
                       http://localhost:2346/mcp, `read_memory`) through the same
                       urllib client tools/duckstation/radio_line.py uses.
  --source states      replay: decode one sample per DuckStation .sav through
                       tools/duckstation/savestate.py (default: every state in the
                       library $AC1_STATES_DIR, ordered by frame number).
  --source jsonl       replay a recording written earlier with --record.
  --source synthetic   a scripted walk through the whole lifecycle, no emulator or
                       game data needed (used to exercise the page).

What is read is driven entirely by browser-tools/plantview/stations.json ("fields" and
"arrays"); the addresses are coalesced into few `read_memory` regions (max 16 KB
each — larger reads time out on the fork). decode_sample() turns the bytes into
the JSON the page draws: raw field values, the 15 entity slots, template counts
and the derived lifecycle state (see "derived" in stations.json for how honest
each of those is).

Endpoints (bind 127.0.0.1 only)
  /                 index.html          /stations.json   the layout
  /state            latest sample       /history?since=N samples with seq >= N
  /sample/N         one sample          /events          SSE stream of new samples
  /info             source, rate, counts

Examples
  python browser-tools/plantview/poller.py --source synthetic
  python browser-tools/plantview/poller.py --source states                # the whole library
  python browser-tools/plantview/poller.py --source states --state ~/dev/ac1_states/M30-mid-01__20261018__v87.sav
  python browser-tools/plantview/poller.py --source live --rate 4 --record out/run.jsonl
"""
from __future__ import annotations
import argparse, glob, json, os, queue, struct, sys, threading, time, webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse, parse_qs

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
DS_DIR = os.path.join(REPO, "tools", "duckstation")
sys.path.insert(0, DS_DIR)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), "_shared"))
import btserve  # noqa: E402  (Server: exclusive port on Windows)
import savestate as SS   # noqa: E402

LAYOUT_DEFAULT = os.path.join(HERE, "stations.json")
MCP_URL_DEFAULT = "http://localhost:2346/mcp"
MAX_REGION = 16 * 1024          # read_memory > ~16 KB times out on the fork
REGION_GAP = 4 * 1024           # merge spans closer than this
RAM_BASE, RAM_SIZE = 0x80000000, 2 * 1024 * 1024

TYPES = {"u8": ("<B", 1), "s8": ("<b", 1), "u16": ("<H", 2), "s16": ("<h", 2),
         "u32": ("<I", 4), "s32": ("<i", 4)}


def _int(v) -> int:
    return v if isinstance(v, int) else int(str(v), 0)


# ---------------------------------------------------------------- layout → regions
def load_layout(path: str) -> dict:
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def spans(layout: dict):
    """(addr, size) for every field and every array in the layout."""
    out = []
    for f in layout.get("fields", {}).values():
        out.append((_int(f["addr"]), TYPES[f["type"]][1]))
    for a in layout.get("arrays", {}).values():
        out.append((_int(a["base"]), _int(a["stride"]) * _int(a["count"])))
    return out


def plan_regions(layout: dict, max_size=MAX_REGION, gap=REGION_GAP):
    """Coalesce spans into few contiguous reads, none larger than max_size."""
    regs = []
    for a, n in sorted(spans(layout)):
        if regs:
            ra, rn = regs[-1]
            if a - (ra + rn) <= gap and max(ra + rn, a + n) - ra <= max_size:
                regs[-1] = (ra, max(ra + rn, a + n) - ra)
                continue
        regs.append((a, n))
    return regs


class RamView:
    """Bytes for a set of (addr -> bytes) regions, addressed by PS1 address."""

    def __init__(self, regions: dict | None = None, flat: bytes | None = None):
        self.regions = regions or {}
        self.flat = flat

    def read(self, addr: int, n: int) -> bytes:
        if self.flat is not None:
            o = addr - RAM_BASE
            return self.flat[o:o + n]
        for ra, data in self.regions.items():
            if ra <= addr and addr + n <= ra + len(data):
                o = addr - ra
                return data[o:o + n]
        raise KeyError(f"{addr:#x}+{n} not in any read region")

    def val(self, addr: int, typ: str):
        fmt, n = TYPES[typ]
        return struct.unpack(fmt, self.read(addr, n))[0]


# ---------------------------------------------------------------- decoding
def decode_sample(layout: dict, ram: RamView, meta: dict) -> dict:
    fields = {}
    for name, f in layout["fields"].items():
        try:
            fields[name] = ram.val(_int(f["addr"]), f["type"])
        except (KeyError, struct.error):
            fields[name] = None

    ents_spec = layout["arrays"]["entities"]
    tmpl_spec = layout["arrays"]["templates"]
    eb, es, ec = _int(ents_spec["base"]), _int(ents_spec["stride"]), _int(ents_spec["count"])
    tb, ts, tc = _int(tmpl_spec["base"]), _int(tmpl_spec["stride"]), _int(tmpl_spec["count"])
    tmpl_lo, tmpl_hi = tb, tb + ts * tc

    def ent_field(base, spec, key):
        o = spec["offsets"][key]
        return ram.val(base + _int(o["off"]), o["type"])

    in_mission = (fields.get("game_mode") == 0 and (fields.get("objobj_ptr") or 0) != 0)
    entities = []
    for i in range(ec):
        b = eb + i * es
        try:
            e = {k: ent_field(b, ents_spec, k) for k in ents_spec["offsets"]}
        except (KeyError, struct.error):
            e = {k: None for k in ents_spec["offsets"]}
        tp = e.get("template_ptr") or 0
        e["slot"] = i
        e["addr"] = b
        e["active"] = bool(in_mission and tmpl_lo <= tp < tmpl_hi and (e.get("think") or 0) != 0)
        e["template_idx"] = (tp - tb) // ts if tmpl_lo <= tp < tmpl_hi else None
        entities.append(e)

    t_total = t_consumed = 0
    if in_mission:
        for i in range(tc):
            b = tb + i * ts
            try:
                hp = ent_field(b, tmpl_spec, "hp")
                cons = ent_field(b, tmpl_spec, "consumed")
            except (KeyError, struct.error):
                break
            if hp:
                t_total += 1
                if cons:
                    t_consumed += 1

    active = [e for e in entities if e["active"]]
    derived = {
        "lifecycle": lifecycle_state(fields, entities),
        "entities_active": len(active),
        "entities_collide": sum(1 for e in active if (e.get("geom_ptr") or 0) != 0),
        "entities_dead": sum(1 for e in active if (e.get("ap") or 0) == 0),
        "templates_total": t_total,
        "templates_consumed": t_consumed,
        "in_mission": in_mission,
    }
    bits = layout["fields"].get("flags", {}).get("bits", {})
    flags = fields.get("flags") or 0
    derived["flag_names"] = [n for m, n in bits.items() if flags & _int(m)]
    player = entities[0] if entities else {}
    derived["player_ap"] = player.get("ap") if player.get("active") else None
    derived["player_ap_max"] = player.get("ap_max") if player.get("active") else None

    s = dict(meta)
    s.update({"fields": fields, "entities": entities, "derived": derived})
    return s


def lifecycle_state(fields: dict, entities: list) -> str:
    """HYPOTHESIS decoder of MISSION_STATE_MACHINE.md §5 from the §3 variables."""
    mode, objobj = fields.get("game_mode"), fields.get("objobj_ptr") or 0
    if mode != 0 or objobj == 0:
        return "OUT_OF_MISSION"
    flags = fields.get("flags") or 0
    endc = fields.get("end_counter") or 0
    if endc:
        return "ENDING"
    if flags & 0x380:                       # a result was committed and the fade is over
        return "EXIT"
    player_bound = bool(entities and entities[0].get("active"))
    if not player_bound:
        return "LOAD"
    if fields.get("objectives_done") == 3:
        return "OBJECTIVES_COMPLETE"
    return "RUNNING"


# ---------------------------------------------------------------- sources
class LiveSource:
    """DuckStation MCP read_memory, one call per planned region."""
    name = "live"

    def __init__(self, layout, url=MCP_URL_DEFAULT, status_every=1, timeout=60.0):
        from radio_line import Mcp     # tools/duckstation — urllib-only client
        self.mcp = Mcp(url=url, timeout=timeout)
        self.mcp.init()
        self.regions = plan_regions(layout)
        self.status_every, self.n = max(1, status_every), 0
        self.emu = {}

    def __iter__(self):
        while True:
            t0 = time.time()
            regs = {}
            for a, n in self.regions:
                regs[a] = self.mcp.read(a, n)
            if self.n % self.status_every == 0:
                try:
                    st = self.mcp.call("get_status")
                    self.emu = {k: st.get(k) for k in ("state", "frame_number", "game_serial")}
                except Exception as ex:       # keep sampling even if status fails
                    self.emu = {"state": f"? ({ex})"}
            self.n += 1
            yield RamView(regs), {"frame": self.emu.get("frame_number"), "emu": self.emu,
                                  "read_ms": int((time.time() - t0) * 1000), "label": "live"}


def resolve_anchor(disc: str | None, lib: str):
    """(anchor_bytes, t_addr) for savestate.py's RAM locator: from $AC1_BIN via the
    states.py disc cache, else from any cached entry in <lib>/disc_cache.json."""
    if disc and os.path.isfile(disc):
        import states as ST
        d = ST.disc_info(disc, lib)
        return bytes.fromhex(d["anchor"]), d["t_addr"]
    cache = os.path.join(lib, "disc_cache.json")
    if os.path.isfile(cache):
        for ent in json.load(open(cache, encoding="utf-8")).values():
            if ent.get("anchor"):
                return bytes.fromhex(ent["anchor"]), ent["t_addr"]
    raise SystemExit("states source needs $AC1_BIN (or --disc) for the EXE RAM anchor, "
                     f"or a cached anchor in {cache}")


class StatesSource:
    """One sample per DuckStation .sav, ordered by the state's frame number."""
    name = "states"

    def __init__(self, paths, disc, lib):
        anchor, t_addr = resolve_anchor(disc, lib)
        self.samples = []
        for p in paths:
            ss = SS.SaveState(p, anchor=anchor, anchor_addr=t_addr)
            fr = SS.frame_numbers(ss.blob)
            self.samples.append((fr[0] if fr else None, p, ss.ram()))
        self.samples.sort(key=lambda s: (s[0] is None, s[0] or 0))

    def __iter__(self):
        for frame, p, ram in self.samples:
            label = os.path.basename(p).split("__")[0]
            yield RamView(flat=ram), {"frame": frame, "label": label, "file": os.path.basename(p)}


class JsonlSource:
    name = "jsonl"

    def __init__(self, path):
        self.path = path

    def __iter__(self):
        with open(self.path, encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line:
                    yield None, json.loads(line)


class SyntheticSource:
    """Scripted walk OUT -> LOAD -> RUNNING -> OBJECTIVES_COMPLETE -> ENDING -> EXIT -> OUT
    on a fake 2 MB RAM, with entities spawning, taking damage and dying."""
    name = "synthetic"

    def __init__(self, layout, loop=True):
        self.L, self.loop = layout, loop
        self.ram = bytearray(RAM_SIZE)

    def w(self, addr, typ, v):
        fmt, n = TYPES[typ]
        struct.pack_into(fmt, self.ram, addr - RAM_BASE, v & ((1 << (8 * n)) - 1) if typ[0] == "u" else v)

    def f(self, name, v):
        fd = self.L["fields"][name]
        self.w(_int(fd["addr"]), fd["type"], v)

    def ent(self, slot, key, v):
        a = self.L["arrays"]["entities"]
        o = a["offsets"][key]
        self.w(_int(a["base"]) + slot * _int(a["stride"]) + _int(o["off"]), o["type"], v)

    def tmpl(self, idx, key, v):
        a = self.L["arrays"]["templates"]
        o = a["offsets"][key]
        self.w(_int(a["base"]) + idx * _int(a["stride"]) + _int(o["off"]), o["type"], v)

    def spawn(self, slot, idx, ap, faction=2):
        tb = _int(self.L["arrays"]["templates"]["base"])
        self.ent(slot, "template_ptr", tb + idx * 44)
        self.ent(slot, "geom_ptr", 0x8019F538 + idx * 44)
        self.ent(slot, "think", 0x801C5D28)
        self.ent(slot, "on_hit", 0x801C5E00)
        self.ent(slot, "faction", faction)
        self.ent(slot, "ap", ap)
        self.ent(slot, "ap_max", ap)
        self.ent(slot, "pos_x", 1000 + slot * 300)
        self.ent(slot, "pos_z", -2000 + slot * 150)
        self.tmpl(idx, "consumed", 1)

    def clear_entities(self):
        a = self.L["arrays"]["entities"]
        b, n = _int(a["base"]) - RAM_BASE, _int(a["stride"]) * _int(a["count"])
        self.ram[b:b + n] = bytes(n)
        t = self.L["arrays"]["templates"]
        b, n = _int(t["base"]) - RAM_BASE, _int(t["stride"]) * _int(t["count"])
        self.ram[b:b + n] = bytes(n)

    def __iter__(self):
        frame = 0
        while True:
            # OUT_OF_MISSION
            self.clear_entities()
            for n in ("objobj_ptr", "stream_ptr", "flags", "end_counter", "timer", "result_code",
                      "objectives_done", "progress_ctr", "terminal_flag"):
                self.f(n, 0)
            self.f("game_mode", 0); self.f("mission_no", 30); self.f("pa_pack", 0)
            self.f("end_anim", 0xFFFF)
            for _ in range(8):
                frame += 1; yield self.snap(frame, "garage")
            # LOAD: objobj bound, templates filled, player not yet bound
            self.f("objobj_ptr", 0x801C4B40); self.f("stream_ptr", 0x80193144); self.f("mission_no_mb", 30)
            for i in range(6):
                self.tmpl(i, "hp", 1200 + i * 100); self.tmpl(i, "type", 3); self.tmpl(i, "resource", 40 + i)
            for _ in range(4):
                frame += 1; yield self.snap(frame, "loading")
            # RUNNING: player + two enemies, then more spawn, damage, deaths
            self.spawn(0, 0, 33536, faction=1)
            self.spawn(1, 1, 2050); self.spawn(2, 2, 3180)
            self.f("timer", 22 * 180)
            ap = 33536
            for k in range(60):
                frame += 1
                self.f("progress_ctr", min(36, k // 2))
                self.f("timer", max(0, 22 * 180 - k * 20))
                if k == 12: self.spawn(3, 3, 1500)
                if k == 20: self.spawn(4, 4, 1600)
                if k == 35: self.spawn(5, 5, 1700)
                if 15 <= k < 40: self.ent(1, "ap", max(0, 2050 - (k - 15) * 90))
                if 25 <= k < 50: self.ent(3, "ap", max(0, 1500 - (k - 25) * 70))
                if k == 45: self.ent(1, "template_ptr", 0); self.ent(1, "think", 0)   # freed
                ap -= 120; self.ent(0, "ap", ap)
                self.f("flags", 0x2000 if k % 8 < 4 else 0)
                yield self.snap(frame, "running")
            # OBJECTIVES_COMPLETE
            self.f("objectives_done", 3); self.f("flags", 0)
            for _ in range(10):
                frame += 1; yield self.snap(frame, "objectives complete")
            # ENDING: border exit commits 0x80 (+0x400), counter 100 -> 0
            self.f("flags", 0x480)
            for c in range(100, -1, -5):
                frame += 1; self.f("end_counter", c); yield self.snap(frame, "ending")
            # EXIT
            self.f("result_code", 0)
            for _ in range(6):
                frame += 1; yield self.snap(frame, "exit")
            if not self.loop:
                return

    def snap(self, frame, phase):
        return RamView(flat=bytes(self.ram)), {"frame": frame, "label": f"synthetic: {phase}"}


# ---------------------------------------------------------------- server
class Hub:
    def __init__(self, history_max: int):
        self.lock = threading.Lock()
        self.history: list[dict] = []
        self.history_max = history_max
        self.clients: list[queue.Queue] = []
        self.seq = 0
        self.info = {}

    def push(self, sample: dict):
        with self.lock:
            sample["seq"] = self.seq
            self.seq += 1
            self.history.append(sample)
            if len(self.history) > self.history_max:
                self.history.pop(0)
            clients = list(self.clients)
        for q in clients:
            try:
                q.put_nowait(sample)
            except queue.Full:
                pass

    def subscribe(self) -> queue.Queue:
        q: queue.Queue = queue.Queue(maxsize=256)
        with self.lock:
            self.clients.append(q)
        return q

    def unsubscribe(self, q):
        with self.lock:
            if q in self.clients:
                self.clients.remove(q)

    def latest(self):
        with self.lock:
            return self.history[-1] if self.history else None

    def since(self, n: int):
        with self.lock:
            return [s for s in self.history if s["seq"] >= n]

    def get(self, n: int):
        with self.lock:
            for s in self.history:
                if s["seq"] == n:
                    return s
        return None


def make_handler(hub: Hub, layout_path: str):
    class H(BaseHTTPRequestHandler):
        def log_message(self, *a):        # quiet
            pass

        def _send(self, code, body: bytes, ctype="application/json"):
            self.send_response(code)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)

        def _json(self, obj, code=200):
            self._send(code, json.dumps(obj).encode())

        def do_GET(self):
            u = urlparse(self.path)
            p = u.path
            if p in ("/", "/index.html"):
                with open(os.path.join(HERE, "index.html"), "rb") as f:
                    return self._send(200, f.read(), "text/html; charset=utf-8")
            if p == "/stations.json":
                with open(layout_path, "rb") as f:
                    return self._send(200, f.read())
            if p == "/state":
                return self._json(hub.latest() or {})
            if p == "/info":
                with hub.lock:
                    n = len(hub.history)
                return self._json(dict(hub.info, history=n, seq=hub.seq))
            if p == "/history":
                n = int(parse_qs(u.query).get("since", ["0"])[0])
                return self._json(hub.since(n))
            if p.startswith("/sample/"):
                s = hub.get(int(p.rsplit("/", 1)[1]))
                return self._json(s or {}, 200 if s else 404)
            if p == "/events":
                return self._sse()
            self._send(404, b"not found", "text/plain")

        def _sse(self):
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            q = hub.subscribe()
            try:
                last = hub.latest()
                if last:
                    self.wfile.write(f"data: {json.dumps(last)}\n\n".encode()); self.wfile.flush()
                while True:
                    try:
                        s = q.get(timeout=15)
                        self.wfile.write(f"data: {json.dumps(s)}\n\n".encode())
                    except queue.Empty:
                        self.wfile.write(b": keepalive\n\n")
                    self.wfile.flush()
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError, OSError):
                pass
            finally:
                hub.unsubscribe(q)
    return H


def sampler(src, layout, hub: Hub, rate: float, record, loop: bool, once: bool):
    period = 1.0 / rate if rate > 0 else 0
    while True:
        prev_lc = None
        for ram, meta in src:
            t0 = time.time()
            if ram is None:                  # jsonl: already decoded
                s = meta
            else:
                s = decode_sample(layout, ram, meta)
            s["source"] = src.name
            s["t"] = time.time()
            lc = s["derived"]["lifecycle"]
            if lc != prev_lc:
                print(f"  [{src.name}] {s.get('label', '')} frame={s.get('frame')} -> {lc}", flush=True)
                prev_lc = lc
            hub.push(s)
            if record:
                record.write(json.dumps(s) + "\n"); record.flush()
            if once:
                return
            dt = period - (time.time() - t0)
            if dt > 0:
                time.sleep(dt)
        if not loop or src.name == "live":
            print("  [replay] end of samples; serving history", flush=True)
            return


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--source", choices=["live", "states", "jsonl", "synthetic"], default="synthetic")
    ap.add_argument("--layout", default=LAYOUT_DEFAULT)
    ap.add_argument("--rate", type=float, default=4.0, help="samples per second (replay/synthetic pacing; live = max)")
    ap.add_argument("--port", type=int, default=8765)
    ap.add_argument("--mcp-url", default=MCP_URL_DEFAULT)
    ap.add_argument("--status-every", type=int, default=1, help="live: call get_status every N samples")
    ap.add_argument("--state", action="append", help="states: a .sav path (repeatable, globs ok); default = whole library")
    ap.add_argument("--lib", default=os.environ.get("AC1_STATES_DIR", os.path.expanduser("~/dev/ac1_states")))
    ap.add_argument("--disc", default=os.environ.get("AC1_BIN"), help="disc .bin for the savestate RAM anchor")
    ap.add_argument("--replay", help="jsonl: recording to replay")
    ap.add_argument("--record", help="write every sample to this .jsonl (keep it outside the repo or in scratch/)")
    ap.add_argument("--history", type=int, default=5000, help="samples kept for the scrubber")
    ap.add_argument("--loop", action="store_true", help="replay: start over when the samples run out")
    ap.add_argument("--once", action="store_true", help="take one sample, print it as JSON and exit (no server)")
    ap.add_argument("--open", action="store_true", help="open the page in the default browser")
    a = ap.parse_args(argv)

    layout = load_layout(a.layout)
    if a.source == "live":
        src = LiveSource(layout, a.mcp_url, a.status_every)
        print(f"live: {len(src.regions)} read_memory regions per sample: "
              + ", ".join(f"{r:#x}+{n}" for r, n in src.regions), file=sys.stderr)
    elif a.source == "states":
        paths = []
        for pat in (a.state or [os.path.join(a.lib, "*.sav")]):
            paths += glob.glob(os.path.expanduser(pat)) or [os.path.expanduser(pat)]
        paths = [p for p in paths if os.path.isfile(p)]
        if not paths:
            ap.error("no .sav files found")
        src = StatesSource(paths, a.disc, a.lib)
        print(f"states: {len(src.samples)} state(s)", file=sys.stderr)
    elif a.source == "jsonl":
        if not a.replay:
            ap.error("--source jsonl needs --replay <file>")
        src = JsonlSource(a.replay)
    else:
        src = SyntheticSource(layout, loop=True)

    hub = Hub(a.history)
    hub.info = {"source": a.source, "rate": a.rate, "layout": os.path.basename(a.layout)}
    if a.once:
        ram, meta = next(iter(src))
        s = meta if ram is None else decode_sample(layout, ram, meta)
        print(json.dumps(s, indent=1))
        return

    url = f"http://127.0.0.1:{a.port}/"
    if btserve.is_running(a.port):          # a second launch: show the running one
        print(f"plant view: already running on {url}")
        if a.open:
            webbrowser.open(url)
        return
    rec = open(a.record, "a", encoding="utf-8") if a.record else None
    threading.Thread(target=sampler, args=(src, layout, hub, a.rate, rec, a.loop, False), daemon=True).start()
    srv = btserve.Server(("127.0.0.1", a.port), make_handler(hub, a.layout))
    print(f"plant view: {url}  (source={a.source}, ctrl-c to stop)", flush=True)
    if a.open:
        webbrowser.open(url)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
