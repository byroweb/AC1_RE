"""cc1 gets float/double literals right (tools/match/patches/gcc-const-double-slots.patch).

    python -m pytest tools/match/tests/test_cc1_float_const.py

The Windows cc1.exe is a 64-bit MinGW build; unpatched, every CONST_DOUBLE lost its
high word to rtx slot padding: `x + 4294967296.0` gave `.word 0, 0`, `x * 1.15` gave
`.word 0x66666666, <garbage>`, `x * 1.5f` gave `.word 0`.  It also sent all of them
to the constant pool, because mips_const_double_ok judged the garbage value; retail
(and the patched build) loads a literal in 1e-300..1e300 (float: 1e-38..1e38) with an
`li.d`/`li.s` immediate, which maspsx expands to lui/ori, and only pools the rest.

Each case runs the vendored cc1 with match_fn.py's flags and checks its asm.  Skipped
(never passed vacuously) without the toolchain.
"""
import os
import re
import struct
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


def compile_asm(cc1, tmp_path, src):
    (tmp_path / "fn.i").write_text(src, encoding="utf-8")
    p = subprocess.run(match_fn.cc1_command(cc1) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"],
                       cwd=tmp_path, capture_output=True, text=True, stdin=subprocess.DEVNULL)
    assert p.returncode == 0, p.stderr
    return (tmp_path / "fn.s").read_text(encoding="utf-8", errors="replace")


def immediates(asm):
    """[(op, value)] of every li.d / li.s in ASM."""
    return [(m.group(1), float(m.group(2)))
            for m in re.finditer(r"^\s*(li\.[ds])\s+\$\d+,(\S+)", asm, re.M)]


def pool_words(asm):
    """The .word values of each `$LCn:` constant-pool entry, in order."""
    pools, cur = [], None
    for line in asm.splitlines():
        line = line.split("#", 1)[0].strip()
        if re.fullmatch(r"\$LC\d+:", line):
            cur = []
            pools.append(cur)
        elif cur is not None and line.startswith(".word"):
            cur.append(int(line.split()[1], 0) & 0xFFFFFFFF)
        elif line:
            cur = None
    return pools


def double_words(v):
    return list(struct.unpack("<II", struct.pack("<d", v)))


def float_word(v):
    return struct.unpack("<I", struct.pack("<f", v))[0]


def f32(v):
    return struct.unpack("<f", struct.pack("<f", v))[0]


@pytest.mark.parametrize("src,want", [
    ("double f(double x) { return x + 4294967296.0; }\n", ("li.d", 4294967296.0)),
    ("double f(double x) { return x * 1.15; }\n", ("li.d", 1.15)),
    ("double f(double x) { return x * -0.1; }\n", ("li.d", -0.1)),
    ("float h(float x) { return x * 1.5f; }\n", ("li.s", 1.5)),
    ("float h(float x) { return x * 0.1f; }\n", ("li.s", f32(0.1))),
])
def test_immediate_constants(cc1, tmp_path, src, want):
    asm = compile_asm(cc1, tmp_path, src)
    assert immediates(asm) == [want]
    assert pool_words(asm) == []


@pytest.mark.parametrize("src,want", [
    ("double f(double x) { return x * 1e305; }\n", double_words(1e305)),
    ("double f(double x) { return x * 1e-305; }\n", double_words(1e-305)),
    ("float h(float x) { return x * 3e38f; }\n", [float_word(3e38)]),
])
def test_constant_pool_words(cc1, tmp_path, src, want):
    assert pool_words(compile_asm(cc1, tmp_path, src)) == [want]


def test_pool_doubles_with_same_low_word_stay_distinct(cc1, tmp_path):
    # 2^1000 and 2^1001 both have a zero low word; with the high word lost they were one entry.
    a, b = 2.0 ** 1000, 2.0 ** 1001
    asm = compile_asm(cc1, tmp_path, "double f(double x) { return x * %r + %r; }\n" % (a, b))
    assert sorted(map(tuple, pool_words(asm))) == sorted([tuple(double_words(a)),
                                                          tuple(double_words(b))])
