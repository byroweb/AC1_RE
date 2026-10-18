#!/usr/bin/env python3
"""
tools/duckstation/radio_line.py - play one XA radio line in the running game
(DuckStation, sadnescity MCP fork) and measure what came out.

This is the in-game half of the dub test in docs/AUDIO_PIPELINE.md section 6: a
take written into a copy of the disc by tools/audio/reinject_xa.py is played by
the game's own XA line player, DuckStation's media capture is decoded, and the
capture is cross-correlated against the take and against the pristine decode of
the channel. Everything the scratch scripts of 2026-10-18 did, in one tool:

  1. STAGE   a library state (tools/duckstation/states.py, disc_map/states/STATES.tsv)
             into a scratch DuckStation slot. DuckStation re-inserts the media named
             in the .sav header on load, so the header's media path is rewritten
             (offset_to_media_path +0xac / media_path_length +0xa8, new string
             appended to the file) to the .cue of the disc copy under test.
  2. BOOT    that .cue (shutting down whatever runs on another image first), load
             the slot, pause.
  3. TRIGGER FUN_8004DB18(id), the game's XA line player: execute-breakpoint on a
             per-frame game function entry (0x8005804C, hit every frame in a
             mission), on the hit save all GPRs (+hi/lo), set a0 = id, ra = hook,
             pc = 0x8004DB18, continue until the hook fires again (the function has
             returned), remove the breakpoint and restore every register. The id is
             xa_file_table_index << 4 | channel; table entries 3..8 are TALK00..05,
             so a manifest clip id (disc_map/voice_manifest.tsv) maps directly.
  4. CAPTURE (optional) DuckStation's `capture` tool (AAC in MP4, MediaFoundation -
             no ffmpeg needed) for N seconds while logging the drive's LBA.
  5. ANALYSE (optional) decode the MP4 (PyAV), resample to 37800 Hz and report the
             normalised cross-correlation (NCC) against --take and against the
             channel decode(s) in disc_map/extracted/AUDIO/channels (or --ref).
  6. CLEAN UP (optional) shut the system down, boot another .cue (the pristine one)
             and delete the scratch slot.

Steps 1-4 need only the standard library (urllib). Step 5 needs numpy, scipy and
PyAV - run the tool with tools/audio/.venv's python for that, or run the
analysis later with --analyse-only.

Typical use (Git Bash, repo root, DuckStation running with its MCP server on
port 2346; the copy made by reinject_xa.py is <image>.cue next to the .bin):

  tools/audio/.venv/Scripts/python tools/duckstation/radio_line.py \\
      --clip TALK00_ch12_c000 --disc ~/dev/ac1_dub.cue --state M30-ctrl-01 --slot 9 \\
      --capture out/dub.mp4 --take take.wav --shutdown --boot-after "${AC1_BIN%.bin}.cue" \\
      --delete-slot
  ... --disc "${AC1_BIN%.bin}.cue" ...                 # same line on the pristine image (reference run)
  ... --id 0x3C ...                             # raw id instead of a clip id
  ... --analyse-only out/dub.mp4 --take take.wav --ref disc_map/extracted/AUDIO/channels/TALK00_ch*.wav
  ... --stage-only                              # just write the slot

The pristine image is never written: the tool only boots it. Save states, disc
images and captures are game data - keep them out of the repo.

MCP facts this relies on (sadnescity DuckStation, build 0.1-12000, 2026-10-18):
  * boot_game needs a Windows absolute path and refuses while a system runs.
  * frame_step pauses at the VBlank exception entry (npc 0x80000084) - PC must not
    be redirected there; a breakpoint on a game function entry is used instead.
  * once a breakpoint has been used, every MCP call takes ~2 s for the rest of the
    emulator session (restoring 35 registers ~ 70 s). Timeouts here allow for it.
"""
from __future__ import annotations
import argparse, csv, hashlib, json, os, shutil, struct, sys, time, urllib.error, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import savestate as SS   # noqa: E402
import states as ST      # noqa: E402

MCP_URL_DEFAULT = "http://localhost:2346/mcp"
VOICE_MANIFEST = os.path.join(REPO, "disc_map", "voice_manifest.tsv")
CHANNELS_DIR = os.path.join(REPO, "disc_map", "extracted", "AUDIO", "channels")

XA_PLAYER = 0x8004DB18          # FUN_8004DB18(u16 id): the XA line player
HOOK_DEFAULT = 0x8005804C       # per-frame game function entry (hit every frame in a mission)
XA_FILE_TABLE = 0x80031764      # 36-byte entries; +32 = CdlLOC of the file
XA_FILE_ENTRY = 36
TALK_FILE_INDEX0 = 3            # entries 3..8 = TALK00..05
RADIO_STATE = 0x80041BBE        # u16 id, flags +2/+3, busy byte +4 (0xFF playing, 0xF0 done)
XA_RATE = 37800
SLOT_MARKER = ".radio_line.json"   # sidecar next to a slot file this tool wrote


class RadioError(Exception):
    pass


# ---------------------------------------------------------------- MCP client
class Mcp:
    """Streamable-HTTP MCP client on urllib only. Calls can take ~2 s each once a
    breakpoint has been used; `timeout` is per HTTP request."""

    def __init__(self, url: str = MCP_URL_DEFAULT, timeout: float = 180.0, verbose: bool = False):
        self.url, self.timeout, self.verbose = url, timeout, verbose
        self.sid = None
        self.n = 0

    def _post(self, body: dict):
        h = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
        if self.sid:
            h["Mcp-Session-Id"] = self.sid
        req = urllib.request.Request(self.url, data=json.dumps(body).encode(), headers=h, method="POST")
        try:
            r = urllib.request.urlopen(req, timeout=self.timeout)
        except urllib.error.URLError as ex:
            raise RadioError(f"DuckStation MCP at {self.url} unreachable ({ex}); is the emulator "
                             f"running with [Debug] EnableMCPServer = true?") from ex
        txt = r.read().decode()
        d = None
        if "event-stream" in r.headers.get("Content-Type", ""):
            for line in txt.splitlines():
                if line.startswith("data:"):
                    d = json.loads(line[5:])
        elif txt.strip():
            d = json.loads(txt)
        return d, r.headers.get("Mcp-Session-Id")

    def init(self):
        d, sid = self._post({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                             "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                                        "clientInfo": {"name": "ac1-radio-line", "version": "1"}}})
        self.sid = sid
        self._post({"jsonrpc": "2.0", "method": "notifications/initialized"})
        info = (d or {}).get("result", {}).get("serverInfo", {})
        if info.get("name") != "duckstation-mcp":
            raise RadioError(f"{self.url} is not the DuckStation MCP server (serverInfo {info})")
        self.n = 1

    def call(self, name: str, args: dict | None = None):
        if self.sid is None:
            self.init()
        self.n += 1
        t0 = time.time()
        d, _ = self._post({"jsonrpc": "2.0", "id": self.n, "method": "tools/call",
                           "params": {"name": name, "arguments": args or {}}})
        if d is None:
            raise RadioError(f"{name}: empty reply")
        if "error" in d:
            raise RadioError(f"{name}{args or ''}: {d['error']}")
        res = d["result"]
        out = []
        for c in res.get("content", []):
            if c.get("type") == "text":
                try:
                    out.append(json.loads(c["text"]))
                except ValueError:
                    out.append(c["text"])
        val = out[0] if len(out) == 1 else out
        if self.verbose:
            print(f"    mcp {name} {args or ''} -> {json.dumps(val)[:160]} ({time.time() - t0:.1f}s)")
        if res.get("isError"):
            raise RadioError(f"{name}{args or ''}: {val}")
        return val

    # convenience
    def state(self) -> str:
        return str(self.call("get_status").get("state", "?"))

    def wait_paused(self, limit: float) -> bool:
        t0 = time.time()
        while time.time() - t0 < limit:
            if self.state() == "paused":
                return True
            time.sleep(0.05)
        return False

    def read(self, addr: int, size: int) -> bytes:
        r = self.call("read_memory", {"address": hex(addr), "size": size})
        data = r.get("data") if isinstance(r, dict) else r
        if isinstance(data, str):
            return bytes.fromhex(data.replace(" ", "").replace("0x", ""))
        return bytes(data)


# ---------------------------------------------------------------- the line id
def msf_bcd_to_lba(b: bytes) -> int:
    def bcd(x):
        return (x >> 4) * 10 + (x & 15)
    m, s, f = bcd(b[0]), bcd(b[1]), bcd(b[2])
    return (m * 60 + s) * 75 + f - 150


def load_manifest_rows(path: str) -> list:
    with open(path, encoding="utf-8", newline="") as f:
        return [dict(r) for r in csv.DictReader(f, delimiter="\t")]


def resolve_line(clip: str | None, raw_id: str | None, manifest: str) -> dict:
    """{'id', 'clip', 'stream', 'file_index', 'channel', 'duration_s', 'talk_lba' (lo, hi)}."""
    if raw_id and clip:
        raise RadioError("give --clip or --id, not both")
    if raw_id:
        v = int(raw_id, 0)
        if not 0 <= v <= 0xFFFE:
            raise RadioError(f"--id {raw_id}: want 0..0xFFFE (file_index << 4 | channel)")
        return {"id": v, "clip": None, "stream": None, "file_index": v >> 4, "channel": v & 15,
                "duration_s": None, "talk_lba": None}
    if not clip:
        raise RadioError("need --clip <manifest clip id> or --id <hex>")
    rows = load_manifest_rows(manifest)
    hit = [r for r in rows if r["clip_id"] == clip]
    if not hit:
        raise RadioError(f"clip {clip!r} not in {manifest}")
    row = hit[0]
    stream = row["stream"]                       # e.g. TALK00_ch12
    if not stream.startswith("TALK"):
        raise RadioError(f"{clip}: only TALK clips have a radio-player id (stream {stream})")
    n = int(stream[4:6])
    ch = int(row["channel"])
    same_file = [r for r in rows if r["source_file"] == row["source_file"]]
    lo = min(int(r["sector_first"]) for r in same_file)
    hi = max(int(r["sector_last"]) for r in same_file)
    return {"id": (TALK_FILE_INDEX0 + n) << 4 | ch, "clip": clip, "stream": stream,
            "file_index": TALK_FILE_INDEX0 + n, "channel": ch,
            "duration_s": float(row["duration_s"]), "talk_lba": (lo, hi),
            "stream_start_lba": int(row["sector_first"]) - ch}


# ---------------------------------------------------------------- staging a state
def _windows_abs(p: str) -> str:
    p = os.path.abspath(p)
    return p.replace("/", "\\") if sys.platform == "win32" else p


def patch_media_path(sav: bytes, new_path: str) -> bytes:
    """Copy of a DUCCS state with its media path replaced (string appended, header re-pointed)."""
    d = bytearray(sav)
    SS.read_header(bytes(d))     # validates the magic
    F = dict(SS.HEADER_FIELDS)
    nb = new_path.encode("utf-8")
    off = len(d)
    d += nb
    struct.pack_into("<I", d, F["offset_to_media_path"], off)
    struct.pack_into("<I", d, F["media_path_length"], len(nb))
    return bytes(d)


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def slot_is_ours(slot_file: str) -> bool:
    """True when the slot file is one this tool wrote (marker sidecar with its sha256)."""
    mk = slot_file + SLOT_MARKER
    if not (os.path.isfile(mk) and os.path.isfile(slot_file)):
        return False
    try:
        return json.load(open(mk, encoding="utf-8")).get("sha256") == ST.sha256_file(slot_file)
    except (OSError, ValueError):
        return False


def stage_state(state_id: str, slot: int, cue: str, states_manifest: str, ds_dir, force: bool) -> str:
    rows = ST.load_manifest(states_manifest)
    row = ST.find_row(rows, state_id)
    lib = ST.library_dir()
    src = os.path.join(lib, row["file"])
    if not os.path.isfile(src):
        raise RadioError(f"{state_id}: library file missing: {src}")
    data = open(src, "rb").read()
    if _sha256(data) != row["sha256"]:
        raise RadioError(f"{state_id}: library file sha256 does not match STATES.tsv - run states.py check")
    dst = ST.slot_path(slot, ds_dir)
    if os.path.isfile(dst) and not force and not slot_is_ours(dst):
        cur = ST.sha256_file(dst)
        if cur not in {r["sha256"] for r in rows}:
            raise RadioError(f"slot {slot} ({dst}) holds a state that is neither archived nor written by "
                             f"this tool; archive it (states.py archive {slot} <id>) or pass --force")
    patched = patch_media_path(data, _windows_abs(cue))
    hdr = SS.read_header(patched)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, "wb") as f:
        f.write(patched)
    json.dump({"tool": "radio_line.py", "state": state_id, "source": src, "media_path": hdr["media_path"],
               "sha256": _sha256(patched), "written": time.strftime("%Y-%m-%d %H:%M:%S")},
              open(dst + SLOT_MARKER, "w", encoding="utf-8"), indent=1)
    print(f"staged {state_id} -> slot {slot} ({dst})\n   media path: {hdr['media_path']}")
    return dst


def delete_slot(slot: int, ds_dir, force: bool) -> None:
    dst = ST.slot_path(slot, ds_dir)
    if not os.path.isfile(dst):
        print(f"slot {slot}: nothing to delete")
        return
    if not force and not slot_is_ours(dst):
        raise RadioError(f"slot {slot} ({dst}) was not written by this tool; not deleting (pass --force)")
    os.remove(dst)
    mk = dst + SLOT_MARKER
    if os.path.isfile(mk):
        os.remove(mk)
    print(f"deleted slot {slot} ({dst})")


# ---------------------------------------------------------------- emulator driving
def _same_path(a: str, b: str) -> bool:
    return os.path.normcase(os.path.abspath(a)) == os.path.normcase(os.path.abspath(b))


def boot_on(mcp: Mcp, cue: str, reboot: bool) -> None:
    st = mcp.call("get_status", {"detail": "full"})
    running = st.get("state") in ("running", "paused")
    cur = st.get("game_path") or ""
    if running and not reboot and cur and _same_path(cur, cue):
        print(f"already booted on {cur} (frame {st.get('frame_number')})")
        return
    if running:
        print(f"shutting down the system on {cur or '?'}")
        mcp.call("shutdown_system", {"save_resume_state": False})
        for _ in range(100):
            if mcp.state() not in ("running", "paused"):
                break
            time.sleep(0.1)
    p = _windows_abs(cue)
    print(f"booting {p}")
    mcp.call("boot_game", {"path": p, "start_paused": True})
    st = mcp.call("get_status", {"detail": "full"})
    if not _same_path(st.get("game_path") or "", cue):
        raise RadioError(f"boot_game did not land on {cue}: status {st}")


def load_slot(mcp: Mcp, slot: int, cue: str) -> None:
    print(f"loading slot {slot}")
    mcp.call("load_state", {"slot": slot})
    mcp.call("pause")
    time.sleep(0.3)
    cd = mcp.call("get_cdrom_state")
    mp = cd.get("media_path") or ""
    if not _same_path(mp, cue):
        raise RadioError(f"after load_state the media is {mp!r}, not {cue!r} - header patch failed?")
    st = mcp.call("get_status")
    print(f"   paused at frame {st.get('frame_number')}, media {mp}")


def check_file_table(mcp: Mcp, line: dict) -> None:
    """Warn if the XA file table entry's CdlLOC is not where the manifest says the stream starts."""
    if line.get("stream_start_lba") is None:
        return
    loc = mcp.read(XA_FILE_TABLE + XA_FILE_ENTRY * line["file_index"] + 32, 4)
    lba = msf_bcd_to_lba(loc)
    ok = lba == line["stream_start_lba"]
    print(f"   XA file table[{line['file_index']}] CdlLOC {loc.hex()} = LBA {lba} "
          f"({'matches' if ok else 'DOES NOT MATCH'} {line['stream']}'s first sector {line['stream_start_lba']})")
    if not ok:
        raise RadioError("the file table does not agree with the manifest; wrong state/disc?")


GPR_SKIP = {"zero", "pc", "npc"}


def trigger_line(mcp: Mcp, line_id: int, hook: int, hit_timeout: float, return_timeout: float) -> dict:
    """Call FUN_8004DB18(line_id) from the hook breakpoint and restore the CPU afterwards."""
    if mcp.state() != "paused":
        mcp.call("pause")
        mcp.wait_paused(10)
    dis = mcp.call("disassemble", {"address": hex(hook), "count": 2})
    print(f"hook {hook:#010x}: " + " ; ".join(i.get("instruction", "?") for i in dis))
    mcp.call("breakpoint", {"action": "add", "type": "execute", "address": hex(hook)})
    try:
        mcp.call("continue")
        if not mcp.wait_paused(hit_timeout):
            raise RadioError(f"hook {hook:#x} not hit within {hit_timeout:.0f} s (not in a mission?)")
        st = mcp.call("get_cpu_execution_state")
        if int(st["pc"], 16) != hook:
            raise RadioError(f"paused at pc {st['pc']}, not at the hook")
        regs = mcp.call("read_registers", {"group": "gpr"})
        print(f"   hook hit: ra {regs['ra']} sp {regs['sp']} ({len(regs)} registers saved)")
        for nm, v in (("a0", line_id), ("ra", hook), ("pc", XA_PLAYER)):
            mcp.call("write_register", {"name": nm, "value": hex(v)})
        t0 = time.time()
        mcp.call("continue")
        if not mcp.wait_paused(return_timeout):
            raise RadioError(f"FUN_{XA_PLAYER:X}({line_id:#x}) did not return to the hook within "
                             f"{return_timeout:.0f} s")
        st = mcp.call("get_cpu_execution_state")
        if int(st["pc"], 16) != hook:
            raise RadioError(f"returned to pc {st['pc']}, not the hook")
        print(f"   FUN_{XA_PLAYER:X}({line_id:#x}) returned in {time.time() - t0:.1f} s")
    finally:
        mcp.call("breakpoint", {"action": "remove", "type": "execute", "address": hex(hook)})
    t0 = time.time()
    n = 0
    for nm, v in regs.items():
        if nm in GPR_SKIP:
            continue
        mcp.call("write_register", {"name": nm, "value": v})
        n += 1
    mcp.call("write_register", {"name": "pc", "value": hex(hook)})
    st = mcp.call("get_cpu_execution_state")
    print(f"   restored {n} registers in {time.time() - t0:.0f} s; pc {st['pc']} npc {st['npc']}")
    rs = mcp.read(RADIO_STATE, 6)
    rid = struct.unpack_from("<H", rs)[0]
    print(f"   radio state {rs.hex()}: id {rid:#06x} busy {rs[4]:#04x}")
    if rid != line_id:
        print(f"   WARNING: the radio id byte is {rid:#x}, not {line_id:#x} - did the player accept it?")
    return {"regs_restored": n, "radio_state": rs.hex(), "radio_id": rid}


def run_and_capture(mcp: Mcp, mp4: str | None, seconds: float, talk_lba) -> dict:
    """Continue for `seconds` (recording to `mp4` if given) and log the drive's position."""
    log = []
    if mp4:
        p = _windows_abs(mp4)
        os.makedirs(os.path.dirname(p) or ".", exist_ok=True)
        r = mcp.call("capture", {"action": "start", "path": p})
        print(f"capture -> {r.get('path', p)}")
    print(f"running {seconds:.1f} s")
    mcp.call("continue")
    t0 = time.time()
    last = None
    in_talk = 0
    while time.time() - t0 < seconds:
        cd = mcp.call("get_cdrom_state")
        lba = cd.get("current_lba")
        t = time.time() - t0
        if lba != last:
            tag = ""
            if talk_lba and lba is not None and talk_lba[0] <= lba <= talk_lba[1]:
                tag = "TALK"
                in_talk += 1
            log.append((round(t, 2), lba, tag))
            last = lba
        time.sleep(0.05)
    mcp.call("pause")
    out = {"lba_log": log, "talk_reads": in_talk}
    if mp4:
        r = mcp.call("capture", {"action": "stop"})
        print(f"capture stopped: {r}")
        out["mp4"] = mp4
    rs = mcp.read(RADIO_STATE, 6)
    out["radio_state_after"] = rs.hex()
    talk = [e for e in log if e[2]]
    shown = talk if len(talk) <= 5 else talk[:3] + talk[-2:]
    print(f"   drive: {len(log)} position changes, {in_talk} inside the stream's sectors"
          + (f"; first/last TALK reads {shown}" if shown else ""))
    print(f"   radio state after: {rs.hex()} (busy byte {rs[4]:#04x})")
    return out


# ---------------------------------------------------------------- analysis
def _np():
    try:
        import numpy as np
        from scipy.signal import fftconvolve, resample_poly
    except ImportError as ex:
        raise RadioError(f"analysis needs numpy + scipy ({ex}); run with tools/audio/.venv's python") from ex
    return np, fftconvolve, resample_poly


def decode_capture(mp4: str):
    """-> (mono float64 array, rate) from DuckStation's AAC/MP4 capture, via PyAV."""
    np, _, _ = _np()
    try:
        import av
    except ImportError as ex:
        raise RadioError(f"decoding the MP4 needs PyAV ({ex}); run with tools/audio/.venv's python") from ex
    c = av.open(mp4)
    s = c.streams.audio[0]
    frames = []
    for fr in c.decode(s):
        a = fr.to_ndarray()
        if a.dtype.kind == "i":
            a = a.astype(np.float64) / float(1 << (8 * a.dtype.itemsize - 1))
        a = a.astype(np.float64)
        a = a.T if fr.format.is_planar else a.reshape(-1, s.channels)
        frames.append(a)
    if not frames:
        raise RadioError(f"{mp4}: no audio decoded")
    x = np.concatenate(frames)
    return x.mean(axis=1), int(s.rate)


def read_wav(path: str):
    """16-bit PCM WAV -> (mono float64, rate) with the stdlib."""
    import wave
    np, _, _ = _np()
    with wave.open(path, "rb") as w:
        if w.getsampwidth() != 2:
            raise RadioError(f"{path}: only 16-bit PCM WAV is read here")
        x = np.frombuffer(w.readframes(w.getnframes()), dtype="<i2").astype(np.float64) / 32768.0
        ch, rate = w.getnchannels(), w.getframerate()
    return x.reshape(-1, ch).mean(axis=1), rate


def to_rate(x, rate: int, target: int):
    from math import gcd
    np, _, resample_poly = _np()
    if rate == target:
        return x
    g = gcd(rate, target)
    return resample_poly(x, target // g, rate // g)


def ncc(cap, ref, rate: int) -> tuple:
    """Peak normalised cross-correlation of ref inside cap -> (ncc, offset_s)."""
    np, fftconvolve, _ = _np()
    cap = cap - cap.mean()
    ref = ref - ref.mean()
    n = len(ref)
    if n > len(cap) or n == 0 or np.sum(ref ** 2) == 0:
        return float("nan"), float("nan")
    xc = fftconvolve(cap, ref[::-1], mode="valid")
    cs = np.concatenate([[0.0], np.cumsum(cap ** 2)])
    e = cs[n:n + len(xc)] - cs[:len(xc)]
    v = xc / np.sqrt(np.maximum(e, 1e-12) * np.sum(ref ** 2))
    i = int(np.argmax(v))
    return float(v[i]), i / rate


def analyse(mp4: str, take: str | None, refs: list, wav_out: str | None = None) -> list:
    np, _, _ = _np()
    cap, rate = decode_capture(mp4)
    print(f"capture {mp4}: {len(cap) / rate:.2f} s at {rate} Hz, rms {np.sqrt(np.mean(cap ** 2)):.4f}")
    cap = to_rate(cap, rate, XA_RATE)
    if wav_out:
        import wave
        with wave.open(wav_out, "wb") as w:
            w.setnchannels(1), w.setsampwidth(2), w.setframerate(XA_RATE)
            w.writeframes((np.clip(cap, -1, 1) * 32767).astype("<i2").tobytes())
        print(f"   decoded capture written to {wav_out} (mono {XA_RATE} Hz)")
    results = []
    for label, p in ([("take", take)] if take else []) + [(os.path.basename(p), p) for p in refs]:
        r, rr = read_wav(p)
        r = to_rate(r, rr, XA_RATE)
        v, off = ncc(cap, r, XA_RATE)
        results.append({"label": label, "path": p, "ncc": v, "at_s": off, "ref_s": len(r) / XA_RATE})
    results.sort(key=lambda d: -(d["ncc"] if d["ncc"] == d["ncc"] else -9))
    print("NCC of each reference against the capture (best first):")
    for d in results[:12]:
        print(f"   ncc={d['ncc']:.3f}  at {d['at_s']:6.2f} s  (ref {d['ref_s']:5.2f} s)  {d['label']}")
    if len(results) > 12:
        print(f"   ... {len(results) - 12} more")
    return results


def default_refs(line: dict, channels_dir: str) -> list:
    """The pristine decode of the line's own channel, plus every other channel of its
    stream so a wrong line would show up as the best match."""
    import glob
    if not line.get("stream"):
        return []
    own = os.path.join(channels_dir, line["stream"] + ".wav")
    others = sorted(glob.glob(os.path.join(channels_dir, line["stream"][:6] + "_ch*.wav")))
    return ([own] if os.path.isfile(own) else []) + [p for p in others if not _same_path(p, own)]


# ---------------------------------------------------------------- main
def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_argument_group("what to play")
    g.add_argument("--clip", help="clip id from disc_map/voice_manifest.tsv, e.g. TALK00_ch12_c000")
    g.add_argument("--id", help="raw radio id instead (hex/int): file_index << 4 | channel, e.g. 0x3C")
    g.add_argument("--manifest", default=VOICE_MANIFEST, help=f"voice manifest (default {VOICE_MANIFEST})")
    g = ap.add_argument_group("where")
    g.add_argument("--disc", help=".cue (or .bin; its .cue is used) to boot - a copy made by reinject_xa.py, "
                              "or the pristine image for a reference run (it is only read)")
    g.add_argument("--state", default="M30-ctrl-01", help="library state id to stage (default M30-ctrl-01, in-mission)")
    g.add_argument("--slot", type=int, default=9, help="scratch DuckStation slot to stage into (default 9)")
    g.add_argument("--states-manifest", default=ST.MANIFEST_DEFAULT)
    g.add_argument("--ds-dir", help="DuckStation data dir (default $AC1_DS_DIR / platform default)")
    g.add_argument("--force", action="store_true", help="overwrite/delete a slot this tool did not write")
    g.add_argument("--url", default=MCP_URL_DEFAULT, help=f"MCP endpoint (default {MCP_URL_DEFAULT})")
    g.add_argument("--hook", default=hex(HOOK_DEFAULT), help=f"per-frame function entry to break on (default {HOOK_DEFAULT:#x})")
    g.add_argument("--reboot", action="store_true", help="shut down and boot even if the same .cue is running")
    g = ap.add_argument_group("what to record")
    g.add_argument("--capture", help="record DuckStation's audio to this .mp4 while the line plays")
    g.add_argument("--seconds", type=float, help="how long to run after the trigger (default clip length + 3 s, min 8)")
    g.add_argument("--take", help="WAV of the take that was dubbed in (reference for the NCC)")
    g.add_argument("--ref", action="append", default=[], help="extra reference WAV(s); default: the stream's "
                                                              "channel decodes in --channels-dir")
    g.add_argument("--channels-dir", default=CHANNELS_DIR)
    g.add_argument("--wav-out", help="also write the decoded, resampled capture as a WAV")
    g.add_argument("--json", help="write the run's facts (ids, NCC table, LBA log) here")
    g = ap.add_argument_group("modes")
    g.add_argument("--stage-only", action="store_true", help="only write the slot file")
    g.add_argument("--no-trigger", action="store_true", help="boot and load, but do not call the player")
    g.add_argument("--analyse-only", metavar="MP4", help="skip the emulator; correlate this capture")
    g = ap.add_argument_group("afterwards")
    g.add_argument("--shutdown", action="store_true", help="shut the system down at the end")
    g.add_argument("--boot-after", metavar="CUE", help="then boot this image (e.g. the pristine .cue), paused")
    g.add_argument("--load-after", type=int, metavar="SLOT", help="and load this slot on it")
    g.add_argument("--delete-slot", action="store_true", help="delete the scratch slot (only one this tool wrote)")
    g.add_argument("-v", "--verbose", action="store_true", help="print every MCP call")
    a = ap.parse_args(argv)

    try:
        return _run(a)
    except (RadioError, ST.StatesError) as ex:
        print(f"error: {ex}", file=sys.stderr)
        return 2


def _run(a) -> int:
    facts = {}
    line = resolve_line(a.clip, a.id, a.manifest) if (a.clip or a.id) else None
    if line:
        facts["line"] = line
        print(f"line: id {line['id']:#06x} = file table entry {line['file_index']} channel {line['channel']}"
              + (f" ({line['clip']}, {line['duration_s']:.2f} s)" if line["clip"] else ""))
    refs = a.ref or (default_refs(line, a.channels_dir) if line else [])

    if a.analyse_only:
        facts["ncc"] = analyse(a.analyse_only, a.take, refs, a.wav_out)
        _dump(a.json, facts)
        return 0

    if not a.disc:
        raise RadioError("--disc is required (the .cue to boot)")
    cue = a.disc if a.disc.lower().endswith(".cue") else os.path.splitext(a.disc)[0] + ".cue"
    if not os.path.isfile(cue):
        raise RadioError(f"{cue}: not found")
    facts["cue"] = os.path.abspath(cue)

    stage_state(a.state, a.slot, cue, a.states_manifest, a.ds_dir, a.force)
    if a.stage_only:
        return 0
    if not line and not a.no_trigger:
        raise RadioError("need --clip or --id (or --no-trigger)")

    mcp = Mcp(a.url, verbose=a.verbose)
    boot_on(mcp, cue, a.reboot)
    load_slot(mcp, a.slot, cue)
    if not a.no_trigger:
        check_file_table(mcp, line)
        hook = int(a.hook, 0)
        facts["trigger"] = trigger_line(mcp, line["id"], hook, hit_timeout=60, return_timeout=120)
        secs = a.seconds or max(8.0, (line["duration_s"] or 5.0) + 3.0)
        facts["run"] = run_and_capture(mcp, a.capture, secs, line["talk_lba"])
        if a.capture:
            try:
                facts["ncc"] = analyse(a.capture, a.take, refs, a.wav_out)
            except RadioError as ex:
                print(f"analysis skipped: {ex}\n   (later: --analyse-only {a.capture} --take ... )")

    if a.shutdown or a.boot_after:
        print("shutting down the system")
        mcp.call("shutdown_system", {"save_resume_state": False})
        for _ in range(100):
            if mcp.state() not in ("running", "paused"):
                break
            time.sleep(0.1)
    if a.boot_after:
        p = _windows_abs(a.boot_after)
        print(f"booting {p} (paused)")
        mcp.call("boot_game", {"path": p, "start_paused": True})
        if a.load_after:
            mcp.call("load_state", {"slot": a.load_after})
            mcp.call("pause")
            print(f"   loaded slot {a.load_after}")
        st = mcp.call("get_status", {"detail": "full"})
        print(f"   now: {st.get('state')} on {st.get('game_path')} frame {st.get('frame_number')}")
    if a.delete_slot:
        delete_slot(a.slot, a.ds_dir, a.force)
    _dump(a.json, facts)
    return 0


def _dump(path, facts):
    if path:
        json.dump(facts, open(path, "w", encoding="utf-8"), indent=1, default=str)
        print(f"facts -> {path}")


if __name__ == "__main__":
    sys.exit(main())
