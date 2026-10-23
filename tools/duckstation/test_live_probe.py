"""tools/duckstation/live_probe.py on synthetic RAM: no emulator needed.

    python -m pytest tools/duckstation/test_live_probe.py

- probe_steps.json loads (every watched region and expectation target exists);
- field decoding (ints, rgb, char[N], arrays) and change listing with loose bytes;
- each diff and sample expectation kind, both verdicts;
- a whole diff step and a whole sample step through Probe with a fake MCP whose RAM
  changes when the "player" presses Enter.
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import live_probe as L  # noqa: E402


def regions():
    return {
        "money": L.Region("money", {"addr": "0x80039CA0", "size": 8,
                                    "fields": [["opts", 0, "u16"], ["credits", 4, "s32"]]}),
        "parts": L.Region("parts", {"addr": "0x80031A58", "size": 12,
                                    "fields": [["core", 0, "u8"], ["head", 2, "u8"], ["owned", 4, "u8[4]"]]}),
        "pad": L.Region("pad", {"addr": "0x801A2550", "size": 32,
                                "fields": [["press", 24, "u32"], ["held", 28, "u32"]]}),
        "names": L.Region("names", {"addr": "0x80031BD4", "size": 36,
                                    "fields": [["ac", 0, "char[18]"], ["pilot", 18, "char[18]"]]}),
        "pal": L.Region("pal", {"addr": "0x80031A64", "size": 12, "fields": [["rgb", 0, "rgb[4]"]]}),
    }


def money(credits, opts=0):
    return struct.pack("<HHi", opts, 0, credits)


def test_spec_file_loads():
    regs, steps = L.load_spec()
    assert {"credits", "equipped", "pad"} <= set(regs)
    assert regs["credits"].field("credits").off == 4
    assert {s["id"] for s in steps} >= {"pad", "buy", "head", "colour", "rename", "mail"}


def test_field_values_and_labels():
    r = regions()
    parts = bytes([0x58, 0, 0x5A, 0, 1, 2, 3, 4, 0, 0, 0, 0])
    assert r["parts"].field("head").value(parts) == 0x5A
    assert r["parts"].field("owned").value(parts) == [1, 2, 3, 4]
    assert r["parts"].label(6) == "owned[2]" and r["parts"].label(9) == "+0x9"
    assert r["pal"].field("rgb").value(bytes(range(12)))[1] == (3, 4, 5)
    assert r["names"].field("ac").fmt(b"Byron\0junk") == '"Byron"'
    assert r["pad"].field("held").fmt(0x40) == "0x00000040"


def test_changes_lists_fields_and_loose_bytes():
    r = regions()["parts"]
    b = bytes(12)
    a = bytearray(b)
    a[2], a[5], a[10] = 7, 1, 9
    assert L.changes(r, b, bytes(a)) == [("head", "0", "7"), ("owned[1]", "0", "1"), ("+0xA", "0x00", "0x09")]


def test_diff_expectations():
    r = regions()
    b = {"money": money(5000), "parts": bytes(12), "names": bytes(36)}
    a = {"money": money(4200), "parts": bytes([0, 0, 0x5B, 0, 0, 1, 0, 0, 0, 0, 0, 0]),
         "names": b"BYRON".ljust(18, b"\0") + bytes(18)}
    chk = lambda e, ans={}: L.check_diff(e, r, b, a, ans)[0]  # noqa: E731
    assert chk({"field": "money.credits", "is": "delta", "value": "-price"}, {"price": "800"}) == "CONFIRMED"
    assert chk({"field": "money.credits", "is": "delta", "value": "-price"}, {"price": "1,000"}) == "REFUTED"
    assert chk({"field": "money.credits", "is": "delta", "value": "-price"}, {}) == "UNCLEAR"
    assert chk({"field": "money.credits", "is": "decreases"}) == "CONFIRMED"
    assert chk({"field": "money.opts", "is": "same"}) == "CONFIRMED"
    assert chk({"field": "parts.head", "is": "changes"}) == "CONFIRMED"
    assert chk({"field": "parts.owned", "is": "one_up"}) == "CONFIRMED"
    assert chk({"field": "parts.owned", "is": "one_down"}) == "REFUTED"
    assert chk({"region": "parts", "is": "only", "fields": ["head"]}) == "REFUTED"
    assert chk({"region": "parts", "is": "only", "fields": ["head", "owned"]}) == "CONFIRMED"
    v, _b, _a, note = L.check_diff({"region": "names", "is": "contains", "value": "name"}, r, b, a, {"name": "Byron"})
    assert v == "CONFIRMED" and note == "at ac"
    assert chk({"region": "names", "is": "contains", "value": "name"}, {"name": "Cohea"}) == "REFUTED"


def test_one_bit_set():
    r = {"mail": L.Region("mail", {"addr": "0x8017F6D0", "size": 4, "fields": [["flags", 0, "u8[4]"]]})}
    e = {"field": "mail.flags", "is": "one_bit_set", "mask": 2}
    assert L.check_diff(e, r, {"mail": bytes([1, 1, 3, 0])}, {"mail": bytes([1, 3, 3, 0])}, {})[:1] == ("CONFIRMED",)
    assert L.check_diff(e, r, {"mail": bytes([1, 1, 3, 0])}, {"mail": bytes([5, 1, 3, 0])}, {})[0] == "REFUTED"


def pad(press, held):
    return bytes(24) + struct.pack("<II", press, held)


def test_sample_expectations():
    r = regions()
    phases = {"idle": [{"pad": pad(0, 0)}] * 5,
              "cross": [{"pad": pad(0x40, 0)}] + [{"pad": pad(0, 0x40)}] * 6}
    v, b, a, note = L.check_sample({"field": "pad.held", "is": "held", "in": "cross"}, r, phases)
    assert (v, b, a) == ("CONFIRMED", "0x00000000", "0x00000040") and "bits 0x40" in note
    assert L.check_sample({"field": "pad.press", "is": "held", "in": "cross"}, r, phases)[0] == "REFUTED"
    assert L.check_sample({"field": "pad.press", "is": "ticks", "in": "cross"}, r, phases)[0] == "CONFIRMED"
    assert L.check_sample({"field": "pad.held", "is": "steady", "in": "idle"}, r, phases)[0] == "CONFIRMED"
    mv = dict(L.moving(r, ["pad"], phases))
    assert mv["pad.held"]["cross"].startswith("0x00000040") and "pad.press" in mv


class FakeMcp:
    """RAM that switches to `after` once the scripted answers say the action was done."""
    def __init__(self, ram):
        self.ram = ram

    def read(self, addr, size):
        return self.ram[addr][:size]


def test_diff_step_end_to_end():
    r = regions()
    mcp = FakeMcp({0x80039CA0: money(5000), 0x80031A58: bytes(12)})
    answers = iter(["", "800"])

    def ask(prompt):
        if "Enter here" in prompt:              # the player buys: RAM changes
            mcp.ram[0x80039CA0] = money(4200)
            mcp.ram[0x80031A58] = bytes([0, 0, 0, 0, 0, 0, 1, 0, 0, 9, 0, 0])
        return next(answers)

    said = []
    p = L.Probe(mcp, r, ask=ask, say=said.append)
    step = {"id": "buy", "kind": "diff", "say": "buy one", "ask": {"price": "price"},
            "watch": ["money", "parts"],
            "expect": [{"field": "money.credits", "is": "delta", "value": "-price"},
                       {"field": "parts.owned", "is": "one_up"}]}
    rows = p.run_step(step)
    verdicts = {(x["field"], x["verdict"]) for x in rows}
    assert ("money.credits", "CONFIRMED") in verdicts and ("parts.owned", "CONFIRMED") in verdicts
    seen = [x for x in rows if x["verdict"] == "seen"]
    assert [x["field"] for x in seen] == ["parts.+0x9"] and seen[0]["addr"] == "0x80031A61"
    assert seen[0]["note"] == "price=800"


def test_sample_step_end_to_end(tmp_path):
    r = regions()
    mcp = FakeMcp({0x801A2550: pad(0, 0)})
    clock = [0.0]
    phase = []

    def ask(prompt):
        phase.append(prompt)
        mcp.ram[0x801A2550] = pad(0, 0x4000 if "cross" in prompt else 0)
        return ""

    def sleep(dt):
        clock[0] += dt

    p = L.Probe(mcp, r, ask=ask, say=lambda s: None, sleep=sleep, clock=lambda: clock[0])
    step = {"id": "pad", "kind": "sample", "watch": ["pad"],
            "phases": [{"label": "idle", "say": "hands off", "seconds": 0.1},
                       {"label": "cross", "say": "hold X", "seconds": 0.1}],
            "expect": [{"field": "pad.held", "is": "held", "in": "cross"}]}
    rows = p.run_step(step)
    assert rows[0]["verdict"] == "CONFIRMED" and rows[0]["after"] == "0x00004000"
    out = tmp_path / "live_probe.tsv"
    L.append_rows(str(out), rows)
    L.append_rows(str(out), rows)
    lines = out.read_text(encoding="utf-8").splitlines()
    assert lines[0].split("\t") == L.OUT_COLS and len(lines) == 1 + 2 * len(rows)
