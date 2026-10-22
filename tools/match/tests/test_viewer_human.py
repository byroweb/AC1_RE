"""tools/loop/human.py and the viewer's control endpoints: the plain-language symptom
classifier, the explanation, the asm -> C line map, versions / undo, the action log,
steer / pause, and (with the toolchain) one lever try end to end.

    python -m pytest tools/match/tests/test_viewer_human.py
"""
import json
import os
import sys
import threading
import time
import urllib.request

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, "tools", "match"))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import heartbeat   # noqa: E402
import hints       # noqa: E402
import human       # noqa: E402
import match_fn    # noqa: E402
import viewer      # noqa: E402
from test_viewer import make_work, post   # noqa: E402


def test_classify_symptoms():
    rows = [[" ", "80000000", "addiu sp,sp,-24", "addiu sp,sp,-24"],
            ["!", "80000004", "addu v0,a0,a1", "addu v1,a0,a1"],            # registers
            ["!", "80000008", "lw v0,16(a0)", "lw v0,20(a0)"],              # constant / offset
            ["!", "8000000c", "beq v0,zero,0x80000020", "bne v0,zero,0x80000020"],   # branch
            ["!", "80000010", "lh v0,0(a1)", "lhu v0,0(a1)"],               # type
            ["!", "80000014", "sw a0,0(s0)", "sw a1,4(s1)"],                # order: a's twin below
            ["!", "80000018", "sw a1,4(s1)", "sw a0,0(s0)"],
            ["!", "8000001c", "jal 0x80001000", "jal 0x80002000"],          # call
            ["!", "80000020", "nop", "addiu v0,v0,1"]]                      # extra
    s = human.classify(rows)
    kinds = {x["kind"]: x["count"] for x in s}
    assert kinds == {"registers": 1, "constant": 1, "branch": 1, "type": 1, "order": 2, "call": 1, "extra": 1}
    assert s[0]["kind"] == "order" and s[0]["first"] == "80000014"
    assert [m["id"] for m in s[0]["moves"]] == ["swap", "permuter"]
    assert all(m["label"] and m["why"] for x in s for m in x["moves"])
    # ours 8 bytes short: two missing instructions on top of the row symptoms
    assert {x["kind"]: x["count"] for x in human.classify(rows[:2], 40, 32)}["extra"] == 2
    assert human.classify([[" ", "1", "nop", "nop"]]) == []


def test_explain():
    s = human.classify([["!", "1", "addu v0,a0,a1", "addu v1,a0,a1"], ["!", "2", "addu v1,a2,a3", "addu v0,a2,a3"]])
    text = human.explain(s, "MISMATCH", 2, 64, 60)
    assert text.startswith("Of the 16 instructions in the retail function, 2 differ in ours.")
    assert "1 instruction(s) shorter" in text and "different registers" in text
    assert "swap neighbouring statements" in text
    assert "byte for byte" in human.explain([], "MATCH")
    assert "does not compile" in human.explain([], "ERROR")


OBJDUMP = """
fn.o:     file format elf32-littlemips

Disassembly of section .text:

00000000 <func_80013430>:
func_80013430():
C:/tmp/kk/fn.i:5
   0:\t27bdffd8 \taddiu\tsp,sp,-40
   4:\tafb3001c \tsw\ts3,28(sp)

00000008 <LM3>:
C:/tmp/kk/fn.i:7
   8:\t8cb00000 \tlw\ts0,0(a1)
C:/tmp/kk/fn.i:8 (discriminator 1)
   c:\t8c820000 \tlw\tv0,0(a0)
"""
FN_I = "typedef int s32;\nextern int g;\n\n\nvoid func_80013430(s32 a, s32 *b) {\n    s32 x;\n    x = *b;\n    g = (*(s32 *)(a));\n}\n"
C_SRC = ('#include "ac1/ac1.h"\n#define F(p) (*(s32 *)(p))\n/* a comment\n   over two lines */\n'
         "void func_80013430(s32 a, s32 *b) {\n    s32 x;\n\n    x = *b;   /* read */\n    g = F(a);\n}\n")


def test_line_map():
    assert human.objdump_lines(OBJDUMP) == {0: 5, 4: 5, 8: 7, 12: 8}
    m = human.line_map(OBJDUMP, FN_I, C_SRC, "func_80013430")
    lines = C_SRC.split("\n")
    assert lines[m[0] - 1].startswith("void func_80013430")
    assert lines[m[8] - 1].strip().startswith("x = *b;")       # comment dropped by cpp: still paired
    assert lines[m[12] - 1].strip() == "g = F(a);"              # macro expanded in fn.i: replace block
    assert human.line_map(OBJDUMP, "nothing here", C_SRC, "func_80013430") == {}


def test_versions_and_actions(tmp_path):
    v = human.Versions(str(tmp_path), "ovl202", "0x8004C25C")
    assert v.list() == [] and v.undo() is None
    assert v.add("int a;\n", "Swap: x", 40, "better", "perm1:perm") == 1
    assert v.add("int b;\n", "Flip: y", 38, "better", "human:1") == 2
    assert [(x["n"], x["score"], x["label"]) for x in v.list()] == [(1, 40, "Swap: x"), (2, 38, "Flip: y")]
    assert v.undo()["n"] == 2 and [x["n"] for x in v.list()] == [1]
    assert v.add("int c;\n", "z", 37, "better", "human:1") == 3        # an undone number is not reused
    assert human.swap_lines("a\nb\nc", 1, 3) == "c\nb\na"
    assert human.swap_lines("a\nb", 1, 1) is None and human.swap_lines("a\nb", 1, 5) is None
    p = str(tmp_path / "actions.tsv")
    human.log_action("exe", "0x80013430", "try", "Swap\tthings\non x", 12, 9, "better", p)
    rows = open(p, encoding="utf-8").read().splitlines()
    assert rows[0].split("\t") == human.ACTION_COLS
    assert rows[1].split("\t")[1:] == ["exe", "0x80013430", "try", "Swap things on x", "12", "9", "better"]


def test_pause_file(tmp_path):
    sub = tmp_path / "sweep"
    sub.mkdir()
    hb = heartbeat.Heartbeat(str(sub), "sweep", 1, interval=0)
    heartbeat.wait_if_paused(str(sub), hb)              # no PAUSE: returns at once
    (tmp_path / "PAUSE").write_text("x")
    t = threading.Thread(target=heartbeat.wait_if_paused, args=(str(sub), hb, 0.05))
    t.start()
    time.sleep(0.3)
    assert t.is_alive() and json.load(open(sub / "current.json"))["paused"]
    os.unlink(tmp_path / "PAUSE")
    t.join(2)
    assert not t.is_alive() and not json.load(open(sub / "current.json"))["paused"]


def serve(app):
    srv = viewer.ThreadingHTTPServer(("127.0.0.1", 0), viewer.make_handler(app))
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv, "http://127.0.0.1:%d" % srv.server_address[1]


def test_steer_pause_endpoints(tmp_path):
    w = make_work(str(tmp_path))
    hp, ap = str(tmp_path / "hints.tsv"), str(tmp_path / "actions.tsv")
    app = viewer.App(w, hints_path=hp, oracle=False, actions_path=ap)
    srv, base = serve(app)
    try:
        code, j = post(base + "/api/steer", {"k": "exe 0x80013430", "what": "agent"})
        assert code == 200 and [(h["kind"], h["text"]) for h in j["hints"]] == [("steer", "agent"), ("triage", "priority")]
        assert post(base + "/api/steer", {"k": "exe 0x80013430", "what": "deep"})[0] == 200
        assert post(base + "/api/steer", {"k": "exe 0x80013430", "what": "later"})[0] == 400
        assert hints.steer(hints.load(hp), "deep") == [("exe", "0x80013430")]
        assert hints.steer(hints.load(hp), "agent") == []                  # the newest steer row wins
        d = json.load(urllib.request.urlopen(base + "/api/func?k=exe+0x80013430"))
        assert d["steer"] == "deep" and d["triage"] == "priority"
        assert {m["id"] for m in d["moves"]} >= {"swap", "commute", "permuter"}
        code, j = post(base + "/api/pause", {"on": True})
        assert code == 200 and j["paused"] and os.path.exists(os.path.join(w, "PAUSE"))
        assert json.load(urllib.request.urlopen(base + "/api/state"))["paused"]
        assert post(base + "/api/pause", {"on": False})[1]["paused"] is False
        code, j = post(base + "/api/try", {"k": "exe 0x80013430", "move": "dance"})
        assert code == 400 and "unknown move" in j["error"]
        code, j = post(base + "/api/record", {"k": "exe 0x80013430"})
        assert code == 400 and "MATCH" in j["error"]
    finally:
        srv.shutdown()
    acts = [ln.split("\t")[3:5] for ln in open(ap, encoding="utf-8").read().splitlines()[1:]]
    assert acts == [["steer", "agent"], ["steer", "deep"], ["pause", "20261021-2003"], ["resume", "20261021-2003"]]


@pytest.fixture(scope="module")
def toolchain():
    if not match_fn.load_toolchain_json():
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    import permute
    try:
        permute.toolchain()
    except match_fn.ToolError as e:
        pytest.skip(str(e))


def test_try_lever_end_to_end(tmp_path, toolchain):
    """A swap lever try on the worked example's perturbed copy: the job scores every swap with
    the oracle's compile, keeps the best as `your try 1`, and its score 0 goes through
    match_fn.py before it is called a MATCH."""
    with open(os.path.join(HERE, "func_8004F7E8_perturbed.c"), encoding="utf-8") as fh:
        text = fh.read()
    tries = human.Tries(str(tmp_path / "human"), str(tmp_path / "actions.tsv"))
    import drive
    row = drive.load_rows().get(("exe", "0x8004F7E8")) or drive.load_rows().get(("ovl201", "0x8004F7E8"))
    if row is None:
        pytest.skip("0x8004F7E8 not in functions.tsv")
    job = tries.submit(row["unit"], "0x8004F7E8", int(row["size"]), text, "test", "swap")
    for _ in range(600):
        j = tries.list()[0]
        if j["status"] in ("done", "failed"):
            break
        time.sleep(0.1)
    r = j["result"]
    assert j["status"] == "done", r
    assert r["before"] > 0 and "outcome" in r
    if r.get("version"):
        v = tries.versions(row["unit"], "0x8004F7E8").list()[-1]
        assert v["score"] == r["after"] and v["result"] == r["outcome"]
        if r["after"] == 0:
            assert r["match"] is (r["outcome"] == "MATCH")
    assert open(str(tmp_path / "actions.tsv"), encoding="utf-8").read().count("\ttry\t") == 1
    assert job["move"] == "swap"
