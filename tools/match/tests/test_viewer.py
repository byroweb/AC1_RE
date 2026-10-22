"""browser-tools/match-viewer/viewer.py, hints.py and heartbeat.py: the parsers, the work-dir model at a
replay time, and the hints round-trip through the HTTP endpoints.

    python -m pytest tools/match/tests/test_viewer.py

No toolchain needed: the viewer runs with its oracle off, the work dir is synthetic.
"""
import datetime
import json
import os
import sys
import threading
import time
import urllib.error
import urllib.request

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
sys.path.insert(0, os.path.join(REPO, "browser-tools", "match-viewer"))
import heartbeat   # noqa: E402
import hints       # noqa: E402
import make_queues  # noqa: E402
import viewer      # noqa: E402

LOG = """
== 20:03:12 overnight pass: work dir /w/20261021-2003, -j 24
branch machine-pass-20261021-2003

== 20:03:13 queue
wrote all1.tsv (601 rows)

== 20:03:13 sweep (ctx-spframe,ctx-fix-frame,neighbour)
[601/601] ovl201  0x8009C964 25504 B  -        ctx-spframe ERROR:too few arguments
sweep: 1/601 bodies matched (0.2%; 601 tried), 1 function(s) credited with twins, 122 s; report r

== 20:05:15 permuter, quick pass (120s each, words/(size/4) <= 0.2)
permute: 1/26 matched (3.8%; 0 by levers alone, 1 by the permuter), 2637 lever compiles + 7396 permuter compiles in 161 s wall = 62.4 compiles/s at -j 24; report r

== 20:07:56 permuter, deep pass (900s each for the 50 closest)
== 23:59:58 twins replay (all scopes): 2 new manifest row(s) so far
== 00:00:05 verify
== 00:04:00 verify did not say all matched: nothing committed, see log
"""


def test_parse_overnight_log():
    day = datetime.date(2026, 10, 21)
    out = viewer.parse_overnight_log(LOG, day)
    kinds = [s["kind"] for s in out["stages"]]
    assert kinds == ["start", "queue", "sweep", "perm1", "perm2", "twins", "verify", "end"]
    assert out["stages"][2]["clock"] == "20:03:13"
    t0 = datetime.datetime(2026, 10, 21, 20, 3, 12).timestamp()
    assert out["stages"][0]["t"] == t0
    # the clock passed midnight: verify is on the next day, after the twins step
    assert out["stages"][6]["t"] - out["stages"][5]["t"] == 7
    assert out["summary"]["sweep"] == {"matched": 1, "total": 601, "tried": 601}
    assert out["summary"]["perm1"]["rate"] == 62.4
    assert viewer.parse_overnight_log(LOG)["stages"][0]["t"] is None


def test_parse_tries():
    t = viewer.parse_tries("ctx-spframe MISMATCH@0x0/86~0.67 | ctx-fix-frame ERROR:+r1 | "
                           "neighbour+imm MISMATCH@0xc/12 | ctx M2C_FAIL:boom | x MATCH")
    assert [(x["variant"], x["result"], x["words"], x["closeness"]) for x in t] == [
        ("ctx-spframe", "MISMATCH", 86, 0.67), ("ctx-fix-frame", "ERROR", None, None),
        ("neighbour+imm", "MISMATCH", 12, None), ("ctx", "M2C_FAIL", None, None), ("x", "MATCH", None, None)]
    assert viewer.parse_tries("-") == [] and viewer.parse_tries(None) == []


def test_parse_logs_and_series():
    lv = viewer.parse_levers_log("   2   12  args f +arg0 proto\n   4   15  swap a / b\n  9   7  x\nnoise\n")
    assert lv == [(2, 12), (4, 15), (9, 7)]
    p = viewer.parse_permuter_log("Loading...\n[f] base score = 40\n\x1b[32;1m[f] found new best score! "
                                  "(10 vs 40)\x1b[0m\n[f] found new best score! (4 vs 10)\n")
    assert p == {"base": 40, "bests": [10, 4]}
    assert viewer.best_so_far([None, 5, 7, 3, 4]) == [None, 5, 5, 3, 3]
    assert viewer.thin(list(range(100)), 5) == [0, 25, 50, 74, 99]
    assert viewer.thin([1, 2], 5) == [1, 2]


def test_parse_oracle_text():
    text = ("fn func_80013430: 224 bytes from x.c, compared with exe\n"
            "resolved symbol(s): func_80014988\n"
            "  addr      retail                             | ours\n"
            "  80013430  addiu sp,sp,-40                    | addiu sp,sp,-40\n"
            "! 8001349c  mflo a1                            | mflo a0\n"
            "m 800134a0  lui at,0x8004                      | lui at,0x0\n"
            "MISMATCH func_80013430 0x6c\n")
    rows, head, last = viewer.parse_oracle_text(text)
    assert rows == [[" ", "80013430", "addiu sp,sp,-40", "addiu sp,sp,-40"],
                    ["!", "8001349c", "mflo a1", "mflo a0"], ["m", "800134a0", "lui at,0x8004", "lui at,0x0"]]
    assert head[0].startswith("fn func_80013430") and last == "MISMATCH func_80013430 0x6c"


def test_tu_map_and_stamp(tmp_path):
    p = tmp_path / "tu.tsv"
    p.write_text("unit\ttu\tstart\tend\tfunctions\tbytes\trodata\tdata\tdoc_systems\n"
                 "exe\texe.t00\t0x80011F28\t0x800125D0\t6\t1704\t\t\tframe loop\n"
                 "exe\texe.t01\t0x800125D0\t0x80012A78\t4\t1192\t\t\tcollision\n")
    m = viewer.load_tu_map(str(p))
    assert viewer.tu_for(m, "exe", "0x800125D0") == ("exe.t01", "collision")
    assert viewer.tu_for(m, "exe", 0x80011F28) == ("exe.t00", "frame loop")
    assert viewer.tu_for(m, "exe", "0x80012A78") == ("", "")
    assert viewer.tu_for(m, "ovl201", "0x80050000") == ("", "")
    assert viewer.stamp_day("20261021-2003") == datetime.date(2026, 10, 21)
    assert viewer.stamp_day("scratch") is None
    real = viewer.load_tu_map()
    assert viewer.tu_for(real, "exe", "0x80011F28")[0] == "exe.t00"


# ---------------------------------------------------------------------------
# a synthetic work dir
# ---------------------------------------------------------------------------
T0 = 1_800_000_000.0


def touch(path, text, t):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)
    os.utime(path, (t, t))


def make_work(root):
    w = os.path.join(root, "20261021-2003")
    touch(os.path.join(w, "overnight.log"), "== 20:03:12 overnight pass\n== 20:03:13 sweep (x)\n", T0)
    touch(os.path.join(w, "all1.tsv"),
          "group\tn_members\tpayoff\ttier\tunit\tvaddr\tname\tsize\n"
          "p1\t3\t0.5\t1\tovl202\t0x80057994\tf\t64\n"
          "p2\t1\t0.4\t1\texe\t0x80013430\tf\t224\n"
          "p3\t1\t0.3\t1\texe\t0x80012048\tf\t356\n", T0)
    sw = os.path.join(w, "sweep")
    touch(os.path.join(sw, "ovl202_80057994_ctx-spframe.c"), "int a;\n", T0 + 10)
    touch(os.path.join(sw, "ovl202_80057994_neighbour.c"), "int b;\n", T0 + 11)
    touch(os.path.join(sw, "exe_80013430_ctx-spframe.c"), "int c;\n", T0 + 20)
    touch(os.path.join(sw, "sweep_report.tsv"),
          "unit\tvaddr\tsize\tgroup\tn_members\tstatus\tcloseness\tpositional\tbest_variant\ttries\tneighbour\n"
          "ovl202\t0x80057994\t64\tp1\t3\tno match\t0.5\t0.1\tctx-spframe\t"
          "ctx-spframe MISMATCH@0x4/14~0.56 | neighbour MISMATCH@0x4/11~0.44\t-\n"
          "exe\t0x80013430\t224\tp2\t1\tno match\t0.9\t0.8\tctx-spframe\tctx-spframe MISMATCH@0x6c/8~0.82\t-\n",
          T0 + 30)
    pd = os.path.join(w, "perm1", "exe_80013430")
    touch(os.path.join(pd, "draft.c"), "void func_80013430(void) { x(); }\n", T0 + 40)
    touch(os.path.join(pd, "meta.json"), "{}", T0 + 40)
    touch(os.path.join(pd, "levers.log"), "   2   12  args\n   3    6  swap\n", T0 + 50)
    touch(os.path.join(pd, "lever.c"), "int pre;\nvoid func_80013430(void) { y(); }\n", T0 + 50)
    touch(os.path.join(pd, "permuter.log"), "[f] base score = 40\n[f] found new best score! (10 vs 40)\n", T0 + 90)
    touch(os.path.join(pd, "output-10-1", "source.c"), "void func_80013430(void) { z(); }\n", T0 + 70)
    os.utime(os.path.join(pd, "output-10-1"), (T0 + 70, T0 + 70))
    touch(os.path.join(w, "perm1", "permute_report.tsv"),
          "unit\tvaddr\tsize\tvariant\twords\tbase\tlever\tlever_evals\tlever_s\tperm_base\tperm_best\t"
          "iterations\tperm_s\tstatus\thow\tnote\n"
          "exe\t0x80013430\t224\tctx-spframe\t8\t8\t6\t3\t10\t40\t10\t400\t40\tno match\t\t\n", T0 + 95)
    return w


def test_workdir_live_and_replay(tmp_path):
    w = make_work(str(tmp_path))
    app = viewer.App(w, hints_path=str(tmp_path / "hints.tsv"), replay=True, oracle=False)
    s = app.state()                                       # t=None: the final state
    f = {x["k"]: x for x in s["funcs"]}
    assert list(f) == ["ovl202 0x80057994", "exe 0x80013430", "exe 0x80012048"]    # queue order
    assert f["ovl202 0x80057994"]["status"] == "tried" and f["ovl202 0x80057994"]["best"] == 11
    assert f["exe 0x80013430"]["status"] == "near"
    assert f["exe 0x80013430"]["best"] == 6                # lever 8 -> 6 (levers.log after the base)
    assert f["exe 0x80013430"]["spark"][:2] == [8, 8]      # sweep words, then the lever series from 8
    assert f["exe 0x80012048"]["status"] == "open"
    assert s["counters"]["tried"] == 2 and s["counters"]["near"] == 1
    assert s["range"][1] == T0 + 90

    s = app.state(T0 + 15)                                # replay: only the first body's drafts
    f = {x["k"]: x for x in s["funcs"]}
    assert f["ovl202 0x80057994"]["status"] == "tried" and f["exe 0x80013430"]["status"] == "open"
    assert s["current"] == "ovl202 0x80057994"
    s = app.state(T0 + 60)                                # exe body in its permuter stage
    assert s["current"] == "exe 0x80013430"
    assert any(a["k"] == "exe 0x80013430" and a["phase"] == "permuter" for a in s["active"])
    assert app.wd.compile_rate(T0 + 60) > 0

    d = app.func("exe 0x80013430")                        # sources: newest permuter text by default
    ids = [x["id"] for x in d["sources"]]
    assert ids == ["sweep:ctx-spframe", "perm1:lever", "perm1:perm"]
    assert d["source"] == "perm1:perm"
    assert d["text"] == "int pre;\nvoid func_80013430(void) { z(); }\n"   # spliced into lever.c
    assert d["series"]["passes"]["perm1"]["levers"] == [8, 8, 6]
    assert d["series"]["passes"]["perm1"]["permuter_base"] == 40
    d = app.func("exe 0x80013430", t=T0 + 45)               # before the lever stage ended
    assert [x["id"] for x in d["sources"]] == ["sweep:ctx-spframe", "perm1:draft"]
    d = app.func("ovl202 0x80057994")                       # sweep only: fewest words wins
    assert d["source"] == "sweep:neighbour" and d["text"] == "int b;\n"


def test_heartbeat_feeds_the_live_view(tmp_path):
    w = make_work(str(tmp_path))
    hb = heartbeat.Heartbeat(os.path.join(w, "perm1"), "permute", 1, interval=0)
    hb.begin("exe", "0x80012048", "levers", size=356)
    hb.add(compiles=5)
    hb.update("exe", "0x80012048", phase="permuter", iterations=12)
    app = viewer.App(w, hints_path=str(tmp_path / "hints.tsv"), oracle=False)
    s = app.state()
    assert s["mode"] == "live"
    assert s["current"] == "exe 0x80012048"
    assert s["active"][0]["phase"] == "permuter" and s["active"][0]["iterations"] == 12
    assert s["heartbeats"]["perm1"]["compiles"] == 5 and not s["heartbeats"]["perm1"]["stale"]
    hb.end("exe", "0x80012048", "MATCH", words=0)
    hb.close()
    app.wd.scanned = 0
    s = app.state()
    assert {x["k"]: x for x in s["funcs"]}["exe 0x80012048"]["status"] == "MATCH"
    data = json.load(open(os.path.join(w, "perm1", "current.json"), encoding="utf-8"))
    assert data["closed"] and data["matches"] == 1 and data["done"]["exe 0x80012048"]["status"] == "MATCH"
    assert heartbeat.Null().begin("x", 1) is None


# ---------------------------------------------------------------------------
# hints: file round-trip, triage, names, the HTTP endpoints, make_queues
# ---------------------------------------------------------------------------
def test_hints_file_roundtrip(tmp_path):
    p = str(tmp_path / "re" / "hints.tsv")
    hints.append("201", "8007cecc", "hint", "draws\tthe radar\n twice", p, "2026-10-21T00:00:00Z")
    hints.append("ovl201", "func_8007CECC", "triage", "priority", p)
    hints.append("exe", "0x80013430", "triage", "skip", p)
    hints.append("exe", "0x80013430", "triage", "none", p)
    rows = hints.load(p)
    assert rows[0] == {"unit": "ovl201", "vaddr": "0x8007CECC", "kind": "hint", "text": "draws the radar twice",
                       "timestamp": "2026-10-21T00:00:00Z"}
    assert open(p, encoding="utf-8").readline() == "unit\tvaddr\tkind\ttext\ttimestamp\n"
    assert hints.triage(rows) == {("ovl201", "0x8007CECC"): "priority"}      # `none` cleared the skip
    assert len(hints.for_function("ovl201", "0x8007cecc", rows)) == 2
    for kind, text in (("triage", "maybe"), ("bogus", "x"), ("hint", "   "), ("name", "two words")):
        with pytest.raises(ValueError):
            hints.append("exe", "0x80013430", kind, text, p)
    assert hints.delete(rows[0], p) and len(hints.load(p)) == 3
    assert not hints.delete(rows[0], p)
    assert hints.body_triage([("ovl202", "0x1"), ("ovl201", "0x8007CECC")], hints.triage(hints.load(p))) == "priority"
    assert hints.body_triage([("ovl202", "0x1")], {("ovl202", "0x00000001"): "skip"}) == "skip"


def test_names_proposals():
    rows = [{"unit": "ovl202", "vaddr": "0x80057994", "kind": "name", "text": "radar_draw", "timestamp": ""},
            {"unit": "ovl202", "vaddr": "0x80057994", "kind": "name", "text": "void parts_loop(AcEntity *e, s32 n)",
             "timestamp": ""},
            {"unit": "ovl201", "vaddr": "0x80050000", "kind": "name", "text": "UiObj *taken(void)", "timestamp": ""},
            {"unit": "exe", "vaddr": "0x80041020", "kind": "name", "text": "int g_x(void)", "timestamp": ""}]
    props = hints.proposals(rows, names={"taken": 0x80050078}, addrs={(0x80041020, "exe"): "g_target_head"})
    by = {p["vaddr"]: p for p in props}
    assert by["0x80057994"]["row"] == "0x80057994\tparts_loop\tvoid (AcEntity *e, s32 n)\t202"   # newest wins
    assert not by["0x80057994"]["problems"]
    assert by["0x80050000"]["ctype"] == "UiObj * (void)" and "already 0x80050078" in by["0x80050000"]["problems"][0]
    assert "already named g_target_head" in by["0x80041020"]["problems"][0]


def post(url, body, origin=None):
    req = urllib.request.Request(url, data=json.dumps(body).encode(), method="POST",
                                 headers={"Content-Type": "application/json", **({"Origin": origin} if origin else {})})
    try:
        with urllib.request.urlopen(req) as r:
            return r.status, json.load(r)
    except urllib.error.HTTPError as e:
        return e.code, json.load(e)


def test_hints_http_roundtrip(tmp_path):
    w = make_work(str(tmp_path))
    hp = str(tmp_path / "hints.tsv")
    app = viewer.App(w, hints_path=hp, oracle=False)
    srv = viewer.ThreadingHTTPServer(("127.0.0.1", 0), viewer.make_handler(app))
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    base = "http://127.0.0.1:%d" % srv.server_address[1]
    try:
        page = urllib.request.urlopen(base + "/").read().decode()
        assert "<title>AC1 match viewer</title>" in page
        code, j = post(base + "/api/hints", {"unit": "exe", "vaddr": "0x80012048", "kind": "hint",
                                             "text": "loops over the 8 AC parts"})
        assert code == 200 and j["ok"] and j["hints"][0]["text"] == "loops over the 8 AC parts"
        code, j = post(base + "/api/hints", {"unit": "exe", "vaddr": "0x80012048", "kind": "triage", "text": "priority"})
        assert code == 200
        code, j = post(base + "/api/hints", {"unit": "exe", "vaddr": "0x80012048", "kind": "triage", "text": "later"})
        assert code == 400 and "triage" in j["error"]
        code, j = post(base + "/api/hints", {"unit": "exe", "vaddr": "0x80012048", "kind": "hint", "text": "x"},
                       origin="http://evil.example")
        assert code == 403
        s = json.load(urllib.request.urlopen(base + "/api/state"))
        f = {x["k"]: x for x in s["funcs"]}["exe 0x80012048"]
        assert f["triage"] == "priority" and f["hints"] == 1
        d = json.load(urllib.request.urlopen(base + "/api/func?k=exe+0x80012048"))
        assert d["triage"] == "priority" and [h["kind"] for h in d["hints"]] == ["hint", "triage"]
        code, j = post(base + "/api/hints/delete", d["hints"][0])
        assert code == 200 and j["ok"] and [h["kind"] for h in j["hints"]] == ["triage"]
        # a hint on a function outside this run still gets a list row
        post(base + "/api/hints", {"unit": "ovl204", "vaddr": "0x80060B08", "kind": "triage", "text": "skip"})
        s = json.load(urllib.request.urlopen(base + "/api/state"))
        assert {x["k"]: x for x in s["funcs"]}["ovl204 0x80060B08"]["outside"]
    finally:
        srv.shutdown()
    rows = hints.load(hp)
    assert [(r["kind"], r["text"]) for r in rows] == [("triage", "priority"), ("triage", "skip")]


def test_make_queues_triage():
    cands = [{"unit": "exe", "vaddr": "0x1", "group": "g1"}, {"unit": "exe", "vaddr": "0x2", "group": "g2"},
             {"unit": "exe", "vaddr": "0x3", "group": "g3"}, {"unit": "exe", "vaddr": "0x4", "group": "g4"}]
    groups = {"g1": [("exe", "0x1")], "g2": [("exe", "0x2")], "g3": [("exe", "0x3"), ("ovl201", "0x9")],
              "g4": [("exe", "0x4")]}
    flags = {("ovl201", "0x00000009"): "priority", ("exe", "0x00000002"): "skip",
             ("exe", "0x00000004"): "priority", ("exe", "0x00000001"): "giveup"}
    out = make_queues.apply_triage(cands, flags, groups)
    assert [c["vaddr"] for c in out] == ["0x3", "0x4"]                 # twin flag counts, order kept
    assert make_queues.apply_triage(cands, {}, groups) is cands
