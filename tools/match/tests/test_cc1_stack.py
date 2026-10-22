"""cc1 survives long straight-line functions (setup.sh links cc1.exe with a 64 MB stack).

    python -m pytest tools/match/tests/test_cc1_stack.py

gcc 2.7.2 recurses over a function's insn chain, so stack use grows with the number of
insns in a basic block.  With MinGW's default 2 MB stack reserve cc1.exe died with no
message (a stack overflow) on a 4000-call straight-line function; the retail shop
constructor ovl201 0x8008C3EC (108 MoveImage calls in a row) was reported crashing
too.  Skipped (never passed vacuously) without the toolchain.
"""
import os
import subprocess
import sys
import types

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
sys.path.insert(0, MATCH_DIR)
import match_fn  # noqa: E402


@pytest.fixture(scope="module")
def cc1():
    tc = match_fn.load_toolchain_json()
    if not tc:
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    try:
        return match_fn.find_cc1(types.SimpleNamespace(cc1=None), tc)
    except match_fn.ToolError as e:
        pytest.skip(str(e))


def test_long_straight_line_function(cc1, tmp_path):
    calls = "".join("    g(%d);\n" % i for i in range(4000))
    (tmp_path / "fn.i").write_text("extern int g(int);\nvoid f(void)\n{\n%s}\n" % calls,
                                   encoding="utf-8")
    p = subprocess.run(match_fn.cc1_command(cc1) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"],
                       cwd=tmp_path, capture_output=True, text=True, stdin=subprocess.DEVNULL)
    assert p.returncode == 0, "cc1 exit %#x (stack overflow?) %s" % (p.returncode & 0xFFFFFFFF, p.stderr)
    assert (tmp_path / "fn.s").read_text(encoding="utf-8", errors="replace").count("jal\tg") == 4000
