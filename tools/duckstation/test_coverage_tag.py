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


def test_expand_units_presets_slots_and_names():
    all_units = ["exe", "ovl201", "ovl202", "ovl203", "ovl204", "fdat000", "fdat160", "fdat180"]
    bases = {"fdat000": 0x801C4B40, "fdat160": 0x801C8460, "fdat180": 0x801B80C8}
    assert C.expand_units(["mission"], all_units, bases) == ["exe", "ovl202", "fdat000"]
    assert C.expand_units(["vs"], all_units, bases) == ["exe", "ovl203", "ovl204", "fdat180", "fdat160"]
    assert C.expand_units(["front", "ovl202", "exe"], all_units, bases) == ["exe", "ovl201", "ovl202"]
    assert C.expand_units(["all"], all_units, bases) == all_units


def test_function_sets_and_names(tmp_path):
    rows = [{"unit": "ovl201", "vaddr": "0x8004ADB0", "is_psyq": 0},
            {"unit": "ovl201", "vaddr": "0x8004ADC0", "is_psyq": 0},
            {"unit": "exe", "vaddr": "0x80012048", "is_psyq": 0},
            {"unit": "exe", "vaddr": "0x80020000", "is_psyq": 1}]
    hints = tmp_path / "hints.tsv"
    hints.write_text("unit\tvaddr\tkind\ttext\ttimestamp\n"
                     "ovl201\t0x8004ADB0\tname\tmail_open\tt\n"
                     "ovl201\t0x8004ADC0\thint\tplay coverage: mail (1)\tt\n", encoding="utf-8")
    syms = tmp_path / "symbols.tsv"
    syms.write_text("# comment\n0x80012048\tac_test_main\tvoid (void)\texe\n"
                    "0x801A2550\tg_pad\tPadState\t201\n", encoding="utf-8")
    named = C.named_functions(str(hints), str(syms))
    assert named == {("ovl201", "0x8004ADB0"), ("exe", "0x80012048")}
    vs = lambda fs, **kw: [r["vaddr"] for r in C.pick_functions(rows, fs, **kw)]  # noqa: E731
    assert vs("all") == ["0x8004ADB0", "0x8004ADC0", "0x80012048"]
    assert vs("unnamed", named=named) == ["0x8004ADC0"]
    assert vs("unseen", covered={("ovl201", "0x8004ADC0")}) == ["0x8004ADB0", "0x80012048"]
    assert vs("open", matched={("exe", "0x80012048")}) == ["0x8004ADB0", "0x8004ADC0"]


def test_resolve_capture():
    rows = [{"unit": "ovl201", "vaddr": "0x8009C964"}, {"unit": "ovl202", "vaddr": "0x8004ADA0"},
            {"unit": "ovl201", "vaddr": "0x8004ADA0"}, {"unit": "exe", "vaddr": "0x80012048"}]
    named = [{"unit": "ovl201", "vaddr": "0x8009C964", "kind": "name", "text": "ac_calc_performance"}]
    assert C.resolve_capture("ac_calc_performance", rows, named) == ("ovl201", 0x8009C964)
    assert C.resolve_capture("0x80012048", rows, named) == ("exe", 0x80012048)
    assert C.resolve_capture("ovl202:0x8004ADA0", rows, named) == ("ovl202", 0x8004ADA0)
    for bad in ("0x8004ADA0", "ovl201:0x8004ADA4", "no_such_name"):
        with pytest.raises(ValueError):
            C.resolve_capture(bad, rows, named)


def test_rearm_skips_known_labels_and_broad_functions():
    u = make_unit()
    hist = {("ovl201", "0x8004ADB0"): {"garage"},
            ("ovl201", "0x8004ADC0"): {"a", "b", "c"}}
    s = C.Session(FakeMcp({}), [u], hits_per_arm=1, log=lambda *a: None, history=hist, max_contexts=3)
    s.context = "garage"
    assert s.arm(u) == 0                                   # one seen under garage, one broad
    s.context = "shop"
    assert s.arm(u) == 1 and list(s.armed) == [0x8004ADB0]


def test_max_armed_queues_fills_on_hit_and_rotates():
    base = 0x8004ADA0
    image = bytearray(0x100)
    fns = [(base + 0x10 * i, 8) for i in range(4)]
    for v, _n in fns:
        image[v - base:v - base + 4] = b"\x08\x00\xe0\x03"
    u = C.Unit("ovl201", bytes(image), base, fns)
    u.probe, u.loaded = 0, True
    fake = FakeMcp({v: u.word(v) for v, _n in fns})
    s = C.Session(fake, [u], hits_per_arm=1, log=lambda *a: None, max_armed=2)
    s.arm(u)
    assert sorted(s.armed) == [base, base + 0x10] and len(s.queue) == 2
    assert s.on_pause(base)                                # hit frees a slot: next one moves in
    assert sorted(s.armed) == [base + 0x10, base + 0x20] and len(s.queue) == 1
    s.rotate()                                             # idle ones swap with the queue
    assert base + 0x30 in s.armed and len(s.armed) == 2
    assert s.bps == set(s.armed)


def test_loaded_now_reads_each_slot_once():
    image_a = bytes([1]) * 0x100
    image_b = bytes([2]) * 0x100
    a = C.Unit("fdat000", image_a, 0x801C4B40, [])
    b = C.Unit("fdat002", image_b, 0x801C4B40, [])
    a.probe, b.probe = 0, 0x40
    ram = bytearray(0x80)
    ram[0x40:0x80] = image_b[0x40:0x80]
    fake = FakeMcp({0x801C4B40: bytes(ram)})
    s = C.Session(fake, [a, b], hits_per_arm=1, log=lambda *x: None)
    assert s.loaded_now() == {"fdat000": False, "fdat002": True}


class RegMcp(FakeMcp):
    """read_registers answers from a list of register dicts, one per call."""
    def __init__(self, ram, regs):
        super().__init__(ram)
        self.regs = list(regs)

    def call(self, name, args=None):
        self.calls.append((name, args))
        if name == "read_registers":
            return self.regs.pop(0)
        return {}


def test_capture_records_args_and_return(tmp_path):
    u = make_unit()
    u.loaded = True
    entry = {"a0": "0x801A2550", "a1": "0x5", "a2": "0x0", "a3": "0x0", "ra": "0x80012510", "sp": "0x801FFF00",
             "v0": "0x0", "v1": "0x0"}
    other = dict(entry, sp="0x801FFE00")                   # same ra, another call frame
    back = dict(entry, v0="0x19C", sp="0x801FFF00")
    ram = {0x8004ADC0: u.word(0x8004ADC0), 0x801A2550: bytes(range(16))}
    m = RegMcp(ram, [entry, other, back])
    s = C.Session(m, [u], hits_per_arm=1, log=lambda *a: None, captures=[("ovl201", 0x8004ADC0)],
                  captures_per_label=1, deref=4)
    s.context = "assembly"
    s.arm(u)
    assert s.on_pause(0x8004ADC0)                          # entry: args read, return bp set
    assert 0x80012510 in s.bps and 0x8004ADC0 not in s.capt
    m.ram[0x801A2550] = b"\xff\xff\xff\xff"                # the callee wrote through a0
    assert s.on_pause(0x80012510) and s.calls == []        # other frame: ours, nothing recorded
    assert s.on_pause(0x80012510)
    (c,) = s.calls
    assert (c["a0"], c["a1"], c["v0"], c["context"]) == ("0x801A2550", "0x00000005", "0x0000019C", "assembly")
    assert c["in"] == "a0:00010203" and c["out"] == "a0:ffffffff"
    assert 0x80012510 not in s.bps and s.new[("ovl201", "0x8004ADC0", "assembly")] == 1
    out = tmp_path / "calls.tsv"
    assert C.append_calls(str(out), s.calls) == 1
    lines = out.read_text(encoding="utf-8").splitlines()
    assert lines[0].split("\t") == C.CALL_COLS and lines[1].split("\t")[7] == "0x0000019C"
