"""Tests for the byte-match oracle.

    python -m pytest tools/match/tests            (or: bash tools/match/tests/run.sh)

They run match_fn.py as a subprocess, exactly as a Claude session would, and
check the exit status plus the final stdout line.  They need the toolchain from
tools/match/setup.sh and the carved overlay 201 (disc_map/overlays/) - without
those they are skipped, never silently passed.
"""
import json
import os
import subprocess
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
MATCH_FN = os.path.join(MATCH_DIR, "match_fn.py")
VENDOR = os.path.join(MATCH_DIR, "vendor")
VADDR = "0x8004F7E8"
GOOD = os.path.join(HERE, "func_8004F7E8.c")
BAD = os.path.join(HERE, "func_8004F7E8_perturbed.c")


def json_part(out):
    """The JSON object printed before the final status line (header/warning lines may precede it)."""
    return json.loads(out[out.index("{"): out.rindex("}") + 1])


def run(*args):
    p = subprocess.run([sys.executable, MATCH_FN, *args], capture_output=True, text=True)
    lines = p.stdout.rstrip("\n").split("\n")
    return p.returncode, p.stdout, lines[-1]


def have_toolchain():
    return os.path.isfile(os.path.join(VENDOR, "toolchain.json"))


@pytest.fixture(scope="module", autouse=True)
def _need_toolchain():
    if not have_toolchain():
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    rc, out, last = run(VADDR, GOOD, "-q")
    if last.startswith("ERROR:") and "overlay" in last:
        pytest.skip("carved overlay 201 not available: " + last)


def test_good_source_matches():
    rc, out, last = run(VADDR, GOOD)
    assert rc == 0, out
    assert last == "MATCH func_8004F7E8 40", out


def test_perturbed_source_mismatches_with_diff():
    rc, out, last = run(VADDR, BAD)
    assert rc == 1, out
    assert last == "MISMATCH func_8004F7E8 0x4", out
    # side-by-side diff: a marked row for the first differing word, both columns present
    assert "! 8004f7ec" in out and "|" in out, out
    assert "lhu v1,68(a0)" in out and "lhu v1,66(a0)" in out, out


def test_json_facts(tmp_path):
    js = tmp_path / "facts.json"
    rc, out, last = run(VADDR, GOOD, "--json", str(js), "-q")
    assert rc == 0 and last == "MATCH func_8004F7E8 40", out
    facts = json.loads(js.read_text())
    assert facts["result"] == "MATCH"
    assert facts["fn"] == "func_8004F7E8"
    assert facts["bytes"] == 40
    assert facts["first_diff_offset"] is None
    assert facts["masked_words"] == 0
    assert facts["bin_label"].startswith("overlay 201")
    assert "-O2" in facts["cc1_flags"] and facts["aspsx_version"] == "2.56"

    rc, out, last = run(VADDR, BAD, "--json", "-", "-q")
    assert rc == 1
    facts = json_part(out)
    assert facts["result"] == "MISMATCH" and facts["first_diff_offset"] == 4 and facts["diff_words"] == 6


def test_expect_size_guard():
    rc, out, last = run(VADDR, GOOD, "--expect-size", "44", "-q")
    assert rc == 1 and last.startswith("MISMATCH func_8004F7E8"), out


def test_missing_source_is_an_error_not_a_mismatch():
    rc, out, last = run(VADDR, os.path.join(HERE, "does_not_exist.c"), "-q")
    assert rc == 2 and last.startswith("ERROR:"), out


def test_external_symbol_masking_and_defsym(tmp_path):
    """ovl201 func_80090C38 reads a global (lui/lw pair).  Without an address the
    two relocated words are masked and reported; with --defsym they are checked."""
    src = tmp_path / "func_80090C38.c"
    src.write_text(
        '#include "ac1_types.h"\n'
        "typedef struct { u8 pad[0x42]; u16 y; } Obj;\n"
        "extern int D_801A2574;\n"
        "void func_80090C38(Obj *o) { o->y -= 32 << D_801A2574; }\n"
    )
    rc, out, last = run("0x80090C38", str(src), "--no-markup", "--json", "-")
    assert rc == 0 and last == "MATCH func_80090C38 32", out
    assert "WARNING: 2 word(s) masked" in out and "D_801A2574" in out, out
    facts = json_part(out)
    assert facts["masked_words"] == 2 and facts["unresolved_symbols"] == ["D_801A2574"]

    rc, out, last = run("0x80090C38", str(src), "--no-markup", "--defsym", "D_801A2574=0x801A2574", "-q")
    assert rc == 0 and last == "MATCH func_80090C38 32", out

    rc, out, last = run("0x80090C38", str(src), "--no-markup", "--defsym", "D_801A2574=0x801A2570")
    assert rc == 1 and last == "MISMATCH func_80090C38 0x8", out
