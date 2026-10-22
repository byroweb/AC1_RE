"""tools/duckstation/coverage_tag.py on synthetic data: no emulator, disc or overlay needed.

    python -m pytest tools/duckstation/test_coverage_tag.py

- MCP reply parsing (JSON and SSE bodies, tool errors);
- probe-window choice between images that share a load address;
- coverage.tsv read / merge / write and the hints.tsv rows (append-only, deduped);
- Session.on_pause with a fake MCP: counting, the --hits cap, a swapped-out overlay,
  and a pause that is not ours.
"""
import json
import os
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import coverage_tag as C  # noqa: E402


def tool_reply(value, is_error=False):
    return {"jsonrpc": "2.0", "id": 2,
            "result": {"content": [{"type": "text", "text": json.dumps(value)}], "isError": is_error}}


def test_parse_body_json_and_sse():
    msg = tool_reply({"status": "running"})
    assert C.parse_body("application/json", json.dumps(msg)) == msg
    sse = "event: message\ndata: %s\n\n" % json.dumps(msg)
    assert C.parse_body("text/event-stream", sse) == msg
    assert C.parse_body("application/json", "") is None


def test_unwrap_values_and_errors():
    assert C.unwrap(tool_reply({"status": "paused", "pc": "0x8004E2BC"}))["pc"] == "0x8004E2BC"
    with pytest.raises(C.McpError):
        C.unwrap(tool_reply({"message": "System not running"}, is_error=True))
    with pytest.raises(C.McpError):
        C.unwrap({"jsonrpc": "2.0", "id": 2, "error": {"code": -32602, "message": "bad"}})


def test_pick_probe_skips_zero_and_shared_windows():
    shared = bytes(range(64))
    a = bytes(64) + shared + b"\x01" * 64
    b = bytes(64) + shared + b"\x02" * 64
    assert C.pick_probe(a, [b]) == 128            # window 0 is zero, window 64 is shared
    assert C.pick_probe(a, [a]) is None
    assert C.pick_probe(a, [b], start=130) == 128         # start rounds down to a window


def test_coverage_roundtrip_and_merge(tmp_path):
    p = str(tmp_path / "coverage.tsv")
    C.write_coverage(p, {("ovl201", "0x8004E2BC", "garage"): 2, ("ovl201", "0x8004DCC0", "shop"): 1})
    got = C.read_coverage(p)
    assert got == {("ovl201", "0x8004E2BC", "garage"): 2, ("ovl201", "0x8004DCC0", "shop"): 1}
    merged = C.merge(got, {("ovl201", "0x8004E2BC", "garage"): 3, ("ovl201", "0x8004E2BC", "combat"): 1})
    assert merged[("ovl201", "0x8004E2BC", "garage")] == 5
    C.write_coverage(p, merged)
    lines = open(p, encoding="utf-8").read().splitlines()
    assert lines[0] == "unit\tvaddr\tcontext\thits"
    assert lines[1].startswith("ovl201\t0x8004DCC0\tshop\t1")    # sorted by address


def test_hints_append_dedupe(tmp_path):
    p = str(tmp_path / "hints.tsv")
    cov = {("ovl201", "0x8004E2BC", "garage"): 3, ("ovl201", "0x8004E2BC", "shop"): 1}
    rows = C.append_hints(p, cov, {("ovl201", "0x8004E2BC")}, now="2026-10-21T00:00:00Z")
    assert rows == [["ovl201", "0x8004E2BC", "hint", "play coverage: garage (3), shop (1)",
                     "2026-10-21T00:00:00Z"]]
    assert C.append_hints(p, cov, {("ovl201", "0x8004E2BC")}) == []          # same summary: no row
    cov[("ovl201", "0x8004E2BC", "combat")] = 9
    assert len(C.append_hints(p, cov, {("ovl201", "0x8004E2BC")})) == 1
    lines = open(p, encoding="utf-8").read().splitlines()
    assert lines[0] == "unit\tvaddr\tkind\ttext\ttimestamp" and len(lines) == 3


class FakeMcp:
    def __init__(self, ram):
        self.ram = ram          # {addr: bytes}
        self.calls = []

    def call(self, name, args=None):
        self.calls.append((name, args))
        return {}

    def read(self, addr, size):
        return self.ram.get(addr, bytes(size))[:size]


def make_unit():
    base = 0x8004ADA0
    image = bytearray(0x100)
    image[0x10:0x14] = b"\x08\x00\xe0\x03"        # jr ra at 0x8004ADB0
    image[0x20:0x24] = b"\xe8\xff\xbd\x27"        # addiu sp,sp,-24 at 0x8004ADC0
    u = C.Unit("ovl201", bytes(image), base, [(0x8004ADB0, 8), (0x8004ADC0, 8)])
    u.probe = 0
    return u


def test_on_pause_counts_caps_and_ignores_foreign_pauses():
    u = make_unit()
    fake = FakeMcp({0x8004ADB0: u.word(0x8004ADB0), 0x8004ADC0: u.word(0x8004ADC0)})
    s = C.Session(fake, [u], hits_per_arm=2, log=lambda *a: None)
    s.arm(u)
    s.context = "garage"
    assert s.on_pause(0x8004ADB0) and s.on_pause(0x8004ADB0)
    assert 0x8004ADB0 not in s.armed                       # cap of 2 reached: removed
    assert ("breakpoint", {"action": "remove", "type": "execute", "address": "0x8004ADB0"}) in fake.calls
    assert s.new == {("ovl201", "0x8004ADB0", "garage"): 2}
    assert s.on_pause(0x80012345) is False                 # not our breakpoint: left paused


def test_on_pause_with_swapped_overlay_counts_nothing():
    u = make_unit()
    fake = FakeMcp({0x8004ADC0: b"\x00\x00\x00\x00"})      # other code at that address now
    s = C.Session(fake, [u], hits_per_arm=1, log=lambda *a: None)
    s.arm(u)
    u.loaded = True
    assert s.on_pause(0x8004ADC0) is True                  # ours: continue the game
    assert s.new == {} and s.armed == {} and u.loaded is False


class LosingMcp(FakeMcp):
    """Fails every call until initialize() is called again (another client took the session)."""
    def __init__(self, ram):
        super().__init__(ram)
        self.lost = True

    def initialize(self):
        self.lost = False
        return {}

    def call(self, name, args=None):
        if self.lost:
            raise C.McpError("HTTP 400: MCP session lost")
        super().call(name, args)
        return {"status": "paused", "pc": "0x8004ADB0"} if name == "wait_for_pause" else {}


def test_cleanup_retakes_a_lost_session_and_resumes():
    u = make_unit()
    s = C.Session(FakeMcp({}), [u], hits_per_arm=1, log=lambda *a: None)
    s.arm(u)
    m = LosingMcp({})
    assert C.cleanup(m, s) is True
    assert s.armed == {}
    assert [c[1]["action"] for c in m.calls if c[0] == "breakpoint"] == ["remove", "remove"]
    assert m.calls[-1][0] == "continue"                    # it was paused on one of ours


def test_summary_collapses_broad_functions():
    cov = {("ovl201", "0x8004E2BC", "screen%02d" % i): 1 for i in range(12)}
    cov[("ovl201", "0x8004E2BC", "garage")] = 5
    assert C.summary(cov, "ovl201", "0x8004E2BC") ==         "play coverage: broad, 13 screens (e.g. garage, screen00, screen01)"
