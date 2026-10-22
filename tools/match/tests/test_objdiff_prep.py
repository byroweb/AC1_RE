"""tools/match/objdiff_prep.py: object writer, path parser, mask handling, config shape.

    python -m pytest tools/match/tests/test_objdiff_prep.py

The toolchain case (the worked example func_8004F7E8 builds the same bytes as its
target) is skipped, never passed vacuously, without tools/match/vendor or the carved
overlay 201.
"""
import os
import struct
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
sys.path.insert(0, MATCH_DIR)
import match_fn      # noqa: E402
import objdiff_prep as P  # noqa: E402


def words(*ws):
    return struct.pack("<%dI" % len(ws), *ws)


def test_elf_object_roundtrip():
    code = words(0x94820040, 0x03E00008, 0x00000000)
    elf = match_fn.Elf(P.elf_object("func_8004F7E8", code))
    assert elf.section_bytes(".text") == code
    name, start, size = match_fn.pick_function(elf, None)
    assert (name, start, size) == ("func_8004F7E8", 0, 12)
    assert elf.data[18] == 8                       # e_machine EM_MIPS
    assert struct.unpack_from("<I", elf.data, 0x24)[0] == 0   # e_flags: mips1 / o32


def test_parse_obj_path_both_separators():
    assert P.parse_obj_path("build/objdiff/base/ovl201/func_8004f7e8.o") == ("base", "ovl201", "0x8004F7E8")
    assert P.parse_obj_path(".\\build\\objdiff\\target\\fdat060\\func_801C4B40.o") == \
        ("target", "fdat060", "0x801C4B40")
    with pytest.raises(P.PrepError):
        P.parse_obj_path("build/objdiff/base/ovl201/foo.o")


def test_unmask_takes_retail_bits_only_under_the_mask():
    ours = words(0x0C000000, 0x3C020000, 0x24420000)
    retail = words(0x0C012345, 0x3C02800B, 0x24435678)
    out = P.unmask(ours, retail, {0: 0x03FFFFFF, 4: 0xFFFF})
    assert out == words(0x0C012345, 0x3C02800B, 0x24420000)   # word 2 not masked: stays ours


def test_diff_words_counts_missing_tail():
    assert P.diff_words(words(1, 2, 3), words(1, 2, 3)) == 0
    assert P.diff_words(words(1, 9), words(1, 2, 3)) == 2


def test_config_shape():
    e = P.unit_entry("ovl201", "0x8004F7E8", os.path.join(P.REPO, "re", "match", "ovl201", "func_8004F7E8.c"), True)
    assert e["name"] == "ovl201/func_8004F7E8"
    assert e["target_path"] == "build/objdiff/target/ovl201/func_8004F7E8.o"
    assert e["base_path"] == "build/objdiff/base/ovl201/func_8004F7E8.o"
    assert e["metadata"] == {"source_path": "re/match/ovl201/func_8004F7E8.c", "complete": True,
                             "progress_categories": ["ovl201"]}
    c = P.make_config([e], ["ovl201"])
    assert c["custom_args"] == ["tools/match/objdiff_prep.py", "build"]
    assert c["build_base"] is True and c["build_target"] is False
    assert c["options"]["mips.instrCategory"] == "r3000gte"


def have_toolchain():
    if not os.path.isfile(os.path.join(MATCH_DIR, "vendor", "toolchain.json")):
        return False
    try:
        match_fn.pick_bin(match_fn.make_parser().parse_args(["0x8004F7E8", "x.c"]), 0x8004F7E8)
        return True
    except match_fn.ToolError:
        return False


@pytest.mark.skipif(not have_toolchain(), reason="needs tools/match/setup.sh and the carved overlay 201")
def test_worked_example_same_bytes_as_target():
    src = os.path.join(HERE, "func_8004F7E8.c")
    bad = os.path.join(HERE, "func_8004F7E8_perturbed.c")
    argv = ["0x8004F7E8", src, "--ovl", "201", "--expect-size", "40"]
    retail = P.retail_bytes(argv)
    assert P.diff_words(P.compile_base(argv)["ours"], retail) == 0
    assert P.diff_words(P.compile_base(["0x8004F7E8", bad, "--ovl", "201", "--expect-size", "40"])["ours"],
                        retail) == 6        # match_fn.py: 6 differing word(s), MISMATCH at 0x4
