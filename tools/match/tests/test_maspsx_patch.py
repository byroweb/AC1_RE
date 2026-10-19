"""maspsx load-delay rule for `l*  $r,%lo(sym)($base)` (tools/match/patches/maspsx-lo-load-nop.patch).

    python -m pytest tools/match/tests/test_maspsx_patch.py

ASPSX does not expand a load whose offset is already `%lo(sym)` off a base register:
it is one machine instruction, so a load into that base register in the slot before
it still needs its load-delay nop.  Unpatched maspsx 670417cf treats the load like the
bare-symbol form (`lw $2,sym($3)`, which does expand through $at) and drops the nop.

Each case runs the vendored maspsx and the mipsel GNU as exactly as match_fn.py does
and checks the assembled words.  Skipped (never passed vacuously) without the toolchain.
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

PATCH = os.path.join(MATCH_DIR, "patches", "maspsx-lo-load-nop.patch")
NOP = 0x00000000


@pytest.fixture(scope="module")
def tools():
    tc = match_fn.load_toolchain_json()
    if not tc:
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    a = types.SimpleNamespace(as_=None, maspsx=None)
    try:
        return match_fn.find_maspsx(a, tc), match_fn.find_as(a, tc)
    except match_fn.ToolError as e:
        pytest.skip(str(e))


def maspsx_lines(tools, tmp_path, body):
    maspsx, _ = tools
    src = tmp_path / "in.s"
    # Framed like cc1 output: maspsx turns `.ent` into `.set noreorder`, so `as` inserts
    # no hazard nops of its own and every nop in the object came from maspsx.
    src.write_text("\t.text\n\t.align\t2\n\t.globl\tf\n\t.ent\tf\nf:\n"
                   + "".join("\t%s\n" % l for l in body) + "\t.end\tf\n", encoding="utf-8")
    p = subprocess.run([sys.executable, maspsx, "--aspsx-version=2.56", str(src)],
                       capture_output=True, text=True, stdin=subprocess.DEVNULL)
    assert p.returncode == 0, p.stderr
    out = []
    for line in p.stdout.splitlines():
        line = line.split("#", 1)[0].strip()
        if line and not line.startswith(".") and not line.endswith(":"):
            out.append(" ".join(line.split()))
    return p.stdout, out


def assemble_words(tools, tmp_path, body):
    """maspsx + as, return the .text words (relocated fields are left as 0 in the object)."""
    maspsx, as_ = tools
    raw, _ = maspsx_lines(tools, tmp_path, body)
    m = tmp_path / "out.s"
    m.write_text(raw, encoding="utf-8")
    o = tmp_path / "out.o"
    p = subprocess.run([as_, "-EL", "-march=r3000", "-mtune=r3000", "-no-pad-sections", "-G0",
                        "-o", str(o), str(m)], capture_output=True, text=True)
    assert p.returncode == 0, p.stderr
    text = match_fn.Elf(o.read_bytes()).section_bytes(".text")
    return [int.from_bytes(text[i:i + 4], "little") for i in range(0, len(text), 4)]


def test_patch_is_applied_to_vendored_maspsx(tools):
    maspsx, _ = tools
    repo = os.path.dirname(maspsx)
    p = subprocess.run(["git", "-C", repo, "apply", "--reverse", "--check", PATCH],
                       capture_output=True, text=True)
    assert p.returncode == 0, "patch not applied, rerun tools/match/setup.sh: " + p.stderr


def test_lw_then_lw_lo_gets_nop(tools, tmp_path):
    # lw $3,0($5) ; nop ; lw $2,%lo(s_attr)($3)
    assert assemble_words(tools, tmp_path, ["lw\t$3,0($5)", "lw\t$2,%lo(s_attr)($3)"]) == \
        [0x8CA30000, NOP, 0x8C620000]


@pytest.mark.parametrize("op,word", [("lhu", 0x94620000), ("lh", 0x84620000),
                                     ("lbu", 0x90620000), ("lb", 0x80620000)])
def test_narrow_lo_loads_get_nop(tools, tmp_path, op, word):
    _, lines = maspsx_lines(tools, tmp_path, ["lw\t$3,0($5)", "%s\t$2,%%lo(s_attr)($3)" % op])
    assert lines == ["lw $3,0($5)", "nop", "%s $2,%%lo(s_attr)($3)" % op]
    assert assemble_words(tools, tmp_path, ["lw\t$3,0($5)", "%s\t$2,%%lo(s_attr)($3)" % op]) == \
        [0x8CA30000, NOP, word]


def test_independent_base_needs_no_nop(tools, tmp_path):
    # the %lo load does not read the loaded register: no hazard, no nop
    _, lines = maspsx_lines(tools, tmp_path, ["lw\t$3,0($5)", "lw\t$2,%lo(s_attr)($4)"])
    assert lines == ["lw $3,0($5)", "lw $2,%lo(s_attr)($4)"]


def test_bare_symbol_form_still_expands_via_at(tools, tmp_path):
    # `lw $2,sym($3)` is a macro: lui $at / addu $at,$at,$3 / lw $2,%lo(sym)($at); the lui
    # does not read $3, so the load delay is covered and no nop is inserted.
    words = assemble_words(tools, tmp_path, ["lw\t$3,0($5)", "lw\t$2,s_attr($3)"])
    assert NOP not in words, ["%08x" % w for w in words]
    assert len(words) == 4 and words[1] >> 16 == 0x3C01, ["%08x" % w for w in words]
