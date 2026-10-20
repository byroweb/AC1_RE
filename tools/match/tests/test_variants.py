"""tools/match/variants.py: edit parsing/applying, and one batch on the worked example.

    python -m pytest tools/match/tests/test_variants.py

- parse_edits() / apply_hunks() on small inputs (text only);
- main() on func_8004F7E8_perturbed.c: the swap that undoes the perturbation is a
  clean oracle MATCH (exit 0, --out written), a no-op edit is not (exit 1, nothing
  written), and the input file is never touched.
Toolchain cases are skipped (never passed vacuously) without tools/match/vendor or the
carved overlay 201.
"""
import os
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
sys.path.insert(0, MATCH_DIR)
import match_fn  # noqa: E402
import variants  # noqa: E402

BAD = os.path.join(HERE, "func_8004F7E8_perturbed.c")
GOOD = os.path.join(HERE, "func_8004F7E8.c")

EDITS = """\
# comment
## swap
-    o->w += 16;
-    o->y += 32;
+    o->y += 32;
+    o->w += 16;
## two hunks
-u16 w;
+s16 w;
-o->x += 64;
+o->x += 65;

## delete
-    o->x += 64;
"""


def test_parse_edits():
    v = variants.parse_edits(EDITS)
    assert [lbl for lbl, _ in v] == ["swap", "two hunks", "delete"]
    assert v[0][1] == [("    o->w += 16;\n    o->y += 32;", "    o->y += 32;\n    o->w += 16;")]
    assert v[1][1] == [("u16 w;", "s16 w;"), ("o->x += 64;", "o->x += 65;")]
    assert v[2][1] == [("    o->x += 64;", "")]


@pytest.mark.parametrize("bad", ["-x\n", "## a\n+x\n", "## a\n## b\n-x\n", "## a\nstray\n", "## a\n"])
def test_parse_edits_rejects(bad):
    with pytest.raises(variants.EditError):
        variants.parse_edits(bad)


def test_apply_hunks():
    text = "a = 1;\n  b  =\t2;\nc = 3;\n"
    assert variants.apply_hunks(text, [("a = 1;", "a = 9;")]) == "a = 9;\n  b  =\t2;\nc = 3;\n"
    # whitespace-tolerant when the exact text is not there
    assert variants.apply_hunks(text, [("b = 2;\nc = 3;", "c = 3;\nb = 2;")]) == "a = 1;\n  c = 3;\nb = 2;\n"
    with pytest.raises(variants.EditError):
        variants.apply_hunks(text, [("d = 4;", "")])
    with pytest.raises(variants.EditError):
        variants.apply_hunks("x;\nx;\n", [("x;", "y;")])


@pytest.fixture(scope="module")
def toolchain():
    if not match_fn.load_toolchain_json():
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    try:
        variants.permute.retail_bytes("ovl201", 0x8004F7E8, 40)
    except Exception as e:      # noqa: BLE001
        pytest.skip("carved overlay 201 not available: %s" % e)


def test_swap_matches(toolchain, tmp_path):
    before = open(BAD, encoding="utf-8").read()
    edits = tmp_path / "e.txt"
    edits.write_text(EDITS)
    out = tmp_path / "best.c"
    rc = variants.main(["0x8004F7E8", BAD, "--ovl", "201", "--edits", str(edits), "--levers", "all",
                        "--out", str(out), "-j", "4"])
    assert rc == 0
    assert open(BAD, encoding="utf-8").read() == before
    def body(s):
        return s.replace("\r", "").split("void func_8004F7E8", 1)[1]
    assert body(out.read_text()) == body(open(GOOD, encoding="utf-8").read())


def test_noop_is_no_match(toolchain, tmp_path, capsys):
    out = tmp_path / "best.c"
    rc = variants.main(["0x8004F7E8", BAD, "--ovl", "201", "--replace", "u16 w;", "s16 w;",
                        "--out", str(out), "-j", "2"])
    assert rc == 1
    assert not out.exists()
    assert "NO MATCH 0x8004F7E8" in capsys.readouterr().out
