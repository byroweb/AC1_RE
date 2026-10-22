"""tools/loop/golem.py against a mocked OpenAI-compatible endpoint (no model needed).

    python -m pytest tools/match/tests/test_golem.py

- answer parsing (edit blocks among chatter, assembly-only blocks dropped; whole-code
  answers spliced), the constant hint, and the request itself (Qwen3 gets /no_think,
  <think> is stripped) - text only;
- tweak on func_8004F7E8_perturbed.c: the mock answers with the swap that undoes the
  perturbation plus a block that does not apply; the job reaches a clean oracle MATCH and
  logs one row;
- fix on a copy with a type cc1 rejects: the mock answers with the whole fixed code; MATCH;
- an endpoint that is down: the job ends with status error, no exception.
Toolchain cases are skipped (never passed vacuously) without tools/match/vendor or the
carved overlay 201.
"""
import http.server
import json
import os
import sys
import threading
import types

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import golem  # noqa: E402

BAD = os.path.join(HERE, "func_8004F7E8_perturbed.c")
GOOD = os.path.join(HERE, "func_8004F7E8.c")

TWEAK_REPLY = """Sure, here are some edits:
```
## swap the last two stores
-    o->w += 16;
-    o->y += 32;
+    o->y += 32;
+    o->w += 16;
## edit the assembly (dropped)
-    lhu v0,66(a0)
+    lhu v0,68(a0)
## not in the file
-    o->z = 1;
+    o->z = 2;
```"""


class Mock:
    """A tiny /v1/chat/completions server: answers from a list, records the requests."""
    def __init__(self, replies):
        self.replies, self.requests = list(replies), []
        mock = self

        class H(http.server.BaseHTTPRequestHandler):
            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                mock.requests.append(body)
                text = mock.replies.pop(0) if mock.replies else "nothing more"
                out = json.dumps({"choices": [{"message": {"role": "assistant", "content": text}}],
                                  "usage": {"prompt_tokens": 100, "completion_tokens": 20}}).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(out)))
                self.end_headers()
                self.wfile.write(out)

            def log_message(self, *a):
                pass

        self.srv = http.server.HTTPServer(("127.0.0.1", 0), H)
        self.url = "http://127.0.0.1:%d/v1" % self.srv.server_address[1]
        threading.Thread(target=self.srv.serve_forever, daemon=True).start()

    def close(self):
        self.srv.shutdown()
        self.srv.server_close()


def args_for(url, **kw):
    a = dict(url=url, model="mock-model", attempts=2, rounds=2, n=4, then_tweak=0, max_words=12,
             max_prompt=golem.MAX_PROMPT, timeout=30, jobs=2, log=None, dump=None)
    a.update(kw)
    return types.SimpleNamespace(**a)


# ---------------------------------------------------------------------------
def test_parse_edit_reply():
    edits = golem.parse_edit_reply(TWEAK_REPLY)
    assert [lbl for lbl, _ in edits] == ["swap the last two stores", "not in the file"]
    assert edits[0][1] == [("    o->w += 16;\n    o->y += 32;", "    o->y += 32;\n    o->w += 16;")]
    assert golem.parse_edit_reply("I cannot help with that.") == []


def test_apply_whole():
    text = '#include "a.h"\n\nint x;\n\nvoid func_80000000(void)\n{\n    x = 1;\n}\n'
    # only the function in the answer: spliced, the declarations above stay
    out = golem.apply_whole(text, "```c\nvoid func_80000000(void)\n{\n    x = 2;\n}\n```", "func_80000000")
    assert out == '#include "a.h"\n\nint x;\n\nvoid func_80000000(void)\n{\n    x = 2;\n}\n'
    # declarations too: the whole code part is replaced, the #include line kept once
    out = golem.apply_whole(text, '```c\n#include "a.h"\nshort x;\nvoid func_80000000(void) { x = 3; }\n```',
                            "func_80000000")
    assert out.count('#include "a.h"') == 1 and "short x;" in out and "int x;" not in out
    assert golem.apply_whole(text, "no code here", "func_80000000") is None


def test_constant_hint():
    rows = [(" ", 0x80000000, "nop", "nop"),
            ("!", 0x80000004, "lui v0,0xa0", "lui v0,0x140"),
            (" ", 0x80000008, "addiu v0,v0,16", "addiu v0,v0,16")]
    h = golem.hint(rows, 12, 12)
    assert "0xA00010" in h and "0x1400010" in h


def test_chat_no_think_and_strip():
    m = Mock(["<think>\nlong musing\n</think>\n## a\n-x;\n+y;"])
    try:
        r = golem.chat(m.url, "qwen/qwen3-8b", "sys", "user", 50)
        assert r["text"] == "## a\n-x;\n+y;"
        assert m.requests[0]["messages"][1]["content"].endswith("/no_think")
        golem.chat(m.url, "qwen2.5-coder-7b-instruct", "sys", "user", 50)
        assert m.requests[1]["messages"][1]["content"] == "user"
    finally:
        m.close()


# ---------------------------------------------------------------------------
@pytest.fixture(scope="module")
def toolchain():
    if not golem.match_fn.load_toolchain_json():
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    try:
        golem.permute.retail_bytes("ovl201", 0x8004F7E8, 40)
    except Exception as e:      # noqa: BLE001
        pytest.skip("carved overlay 201 not available: %s" % e)


def test_tweak_reaches_match(toolchain, tmp_path):
    m = Mock([TWEAK_REPLY])
    log = tmp_path / "log.tsv"
    try:
        tgt = golem.Target("ovl201", 0x8004F7E8, BAD)
        res = golem.do_tweak(tgt, args_for(m.url), "mock-model", golem.Log(str(log)), str(tmp_path))
    finally:
        m.close()
    assert res["status"] == "MATCH" and res["before"] > 0 and res["after"] == 0
    assert res["calls"] == 1
    prompt = m.requests[0]["messages"][1]["content"]
    assert "o->w += 16;" in prompt and "lhu" in prompt          # the C and the listing
    rows = log.read_text().splitlines()
    assert rows[0].split("\t") == golem.LOG_COLS and len(rows) == 2 and "\tMATCH\t" in rows[1]


def test_fix_reaches_match(toolchain, tmp_path):
    broken = tmp_path / "func_8004F7E8.c"
    good = open(GOOD, encoding="utf-8").read()
    broken.write_text(good.replace("void func_8004F7E8(Func8004F7E8Obj *o)",
                                   "void func_8004F7E8(Func8004F7E8Obj o)"))
    fixed = good.split('#include "ac1_types.h"', 1)[1]
    m = Mock(["```c\n" + fixed + "```"])
    try:
        tgt = golem.Target("ovl201", 0x8004F7E8, str(broken))
        assert tgt.evaluate(tgt.text)["status"] == "ERROR"
        res = golem.do_fix(tgt, args_for(m.url, dump=str(tmp_path / "dump")), "mock-model", golem.Log(None),
                           str(tmp_path))
    finally:
        m.close()
    assert res["status"] == "MATCH", res["note"]
    prompt = m.requests[0]["messages"][1]["content"]
    assert "invalid type argument" in prompt or "->" in prompt
    assert len(os.listdir(tmp_path / "dump")) == 2                # prompt + answer


def test_endpoint_down(toolchain, tmp_path):
    tgt = golem.Target("ovl201", 0x8004F7E8, BAD)
    res = golem.do_tweak(tgt, args_for("http://127.0.0.1:9/v1", timeout=5), "mock-model", golem.Log(None),
                         str(tmp_path))
    assert res["status"] == "no match" and res["calls"] == 0
