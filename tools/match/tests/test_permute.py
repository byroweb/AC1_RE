"""tools/loop/permute.py plumbing: the permuter's compile step must be the oracle's.

    python -m pytest tools/match/tests/test_permute.py

- maspsx_inproc() gives the same text as the vendored maspsx.py subprocess (line ends aside);
- compile_candidate() on the worked example gives exactly the oracle's words (relocations
  resolved the way match_fn.py resolves them, .rel.text dropped);
- splice() / find_function() / report_candidates() on small inputs.
Toolchain cases are skipped (never passed vacuously) without tools/match/vendor.
"""
import os
import struct
import subprocess
import sys
import types

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
REPO = os.path.dirname(os.path.dirname(MATCH_DIR))
sys.path.insert(0, MATCH_DIR)
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import match_fn  # noqa: E402
import permute   # noqa: E402

EXAMPLE = os.path.join(HERE, "func_8004F7E8.c")
EXAMPLE_VADDR = 0x8004F7E8


@pytest.fixture(scope="module")
def tools():
    if not match_fn.load_toolchain_json():
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    try:
        return permute.toolchain()
    except match_fn.ToolError as e:
        pytest.skip(str(e))


def test_maspsx_inproc_equals_subprocess(tools, tmp_path):
    work = str(tmp_path)
    i = os.path.join(work, "fn.i")
    permute.preprocess(tools, EXAMPLE, i)
    permute.run(match_fn.cc1_command(tools["cc1"]) + ["fn.i"] + match_fn.CC1_FLAGS + ["-o", "fn.s"], work)
    with open(os.path.join(work, "fn.s"), encoding="utf-8") as fh:
        asm = fh.read()
    sub = subprocess.run([sys.executable, tools["maspsx"], "--aspsx-version=" + match_fn.ASPSX_VERSION,
                          "fn.s"], cwd=work, stdin=subprocess.DEVNULL, capture_output=True, text=True,
                         check=True).stdout
    assert permute.maspsx_inproc(tools["maspsx"], asm).splitlines() == sub.splitlines()


def text_words(obj):
    with open(obj, "rb") as fh:
        elf = match_fn.Elf(fh.read())
    name, start, size = match_fn.pick_function(elf, None)
    return elf, elf.section_bytes(".text")[start:start + size]


def test_compile_candidate_is_the_oracle(tools, tmp_path):
    work = str(tmp_path)
    i = os.path.join(work, "fn.i")
    permute.preprocess(tools, EXAMPLE, i)
    meta = {"fn": None, "vaddr": "0x%08X" % EXAMPLE_VADDR, "rodata": None, "syms": [],
            "defsyms": [], "tools": tools}
    out = os.path.join(work, "cand.o")
    with open(i, encoding="utf-8") as fh:
        assert permute.compile_candidate(meta, {}, fh.read(), out, show_errors=True)
    elf, words = text_words(out)
    assert ".rel.text" not in elf.by_name or elf.by_name[".rel.text"]["type"] != 9, ".rel.text must be dropped"
    # the oracle's own bytes for the same source
    ns = types.SimpleNamespace(include=None, define=None, no_rodata_align_fix=False,
                               aspsx_version=match_fn.ASPSX_VERSION)
    obj, _ = match_fn.compile_to_object(ns, tools, EXAMPLE, os.path.join(work, ""), [])
    with open(obj, "rb") as fh:
        e2 = match_fn.Elf(fh.read())
    _n, start, size = match_fn.pick_function(e2, None)
    want, _m, _r, _u = match_fn.resolve_relocs(e2, start, size, EXAMPLE_VADDR - start, {}, None, None)
    assert words == want


def test_find_function_skips_prototypes():
    text = "void f(int);\nint g(void) { return 1; }\nvoid f(int a)\n{\n  if (a) { g(); }\n}\nint h;\n"
    a, b = permute.find_function(text, "f")
    assert text[a:b].startswith("void f(int a)") and text[a:b].endswith("}\n}")


def test_splice_replaces_only_the_function():
    draft = '#include "ac1/ac1.h"\nextern s32 D_1;\nvoid func_X(void) {\n    D_1 = 1;\n}\n'
    perm = "typedef int s32;\nextern s32 D_1;\nvoid func_X(void)\n{\n  D_1 = 2;\n}\n\n"
    out = permute.splice(draft, perm, "func_X")
    assert out == '#include "ac1/ac1.h"\nextern s32 D_1;\nvoid func_X(void)\n{\n  D_1 = 2;\n}\n'
    assert permute.splice(draft, "int y;", "func_X") is None


def test_report_candidates_takes_best_mismatch(tmp_path):
    p = tmp_path / "sweep_report.tsv"
    p.write_text("unit\tvaddr\tsize\tgroup\tn_members\tstatus\ttries\n"
                 "ovl201\t0x80086FD4\t40\tp1\t3\tno match\tctx MISMATCH@0xc/4 | ctx-regvars MISMATCH@0x8/1\n"
                 "ovl201\t0x80086000\t40\tp2\t1\tMATCH\tctx MATCH\n"
                 "exe\t0x80012000\t64\tp3\t1\tno match\tctx ERROR:cc1.exe failed\n", encoding="utf-8")
    c = permute.report_candidates([str(p)])
    assert list(c) == [("ovl201", "0x80086FD4")]
    assert (c[("ovl201", "0x80086FD4")]["variant"], c[("ovl201", "0x80086FD4")]["words"]) == ("ctx-regvars", 1)
    assert c[("ovl201", "0x80086FD4")]["n_members"] == 3


def test_target_object_holds_the_words(tools, tmp_path):
    blob = struct.pack("<3I", 0x2402003F, 0x03E00008, 0)
    permute.write_target(tools, "func_X", blob, str(tmp_path))
    _elf, words = text_words(os.path.join(str(tmp_path), "target.o"))
    assert words == blob


# --- tools/loop/levers.py ---------------------------------------------------
import levers  # noqa: E402

INDEXED = ('#include "ac1/ac1.h"\n\nvoid func_80086FD4(void) {\n    s32 var_v0;\n\n    var_v0 = 0x3F;\n'
           '    do {\n        M2C_FIELD((var_v0 + 0x801F0000), s8 *, 0x37D4) = -1;\n        var_v0 -= 1;\n'
           '    } while (var_v0 != -1);\n}\n')


def test_lever_index_names_the_array():
    labels = dict(levers.lever_index(INDEXED, "func_80086FD4"))
    t = labels["index D_801F37D4 s8[]"]
    assert "extern s8 D_801F37D4[];\nvoid func_80086FD4(void) {" in t
    assert "D_801F37D4[var_v0] = -1;" in t
    assert "(*(s8 *)(D_801F37D4 + (var_v0)))" in labels["index D_801F37D4 u8[]"]


def test_lever_index_scaled_index():
    t = ("void func_80084A8C(s32 arg0) {\n    f(M2C_FIELD((((g(arg0) & 0xFF) * 4) + 0x800C0000), s32 *, -0x75D0), 4);\n}\n")
    labels = dict(levers.lever_index(t, "func_80084A8C"))
    assert "f(D_800B8A30[g(arg0) & 0xFF], 4);" in labels["index D_800B8A30 s32[]"]


def test_lever_const_and_types():
    t = "extern M2C_UNK D_8004A268;\nvoid func_X(void) {\n    s8 *var_v0;\n    *var_v0 = 0xFF;\n    x = a - 1;\n}\n"
    got = [lbl for lbl, _ in levers.variants(t, "func_X", "const,ptype,etype")]
    assert "const 0xFF->-1" in got and not any("->0xFF" in g for g in got)   # `a - 1` is subtraction
    assert "ptype var_v0 s8->u8" in got and "etype D_8004A268 M2C_UNK->u8" in got


def test_lever_swap_and_commute():
    t = "void func_X(void) {\n    a = b | c;\n    d = 1;\n    return;\n}\n"
    got = dict(levers.variants(t, "func_X", "swap,commute"))
    assert any(v == "void func_X(void) {\n    d = 1;\n    a = b | c;\n    return;\n}\n" for v in got.values())
    assert any("a = c | b;" in v for v in got.values())


def test_lever_bytes():
    t = ("extern M2C_UNK D_80049E74;\nvoid func_X(s32 *arg0) {\n    s16 *var_s1;\n\n    var_s1 += 8;\n"
         "    *D_1 = arg0 + 4;\n    *(i + &D_80049E74) = 0;\n}\n")
    got = dict(levers.lever_bytes(t, "func_X"))
    assert "    var_s1 += 4;\n" in got["bytes var_s1 += 4"]
    assert "var_s1 = (s16 *)((u8 *)var_s1 + 8);" in got["bytes var_s1 += 8"]
    assert "*D_1 = (arg0 + 1);" in got["bytes arg0 + 1"]
    assert "*(((M2C_UNK *)((u8 *)&D_80049E74 + i))) = 0;" in got["bytes E + &D_80049E74"]


def test_lever_args():
    t = ("M2C_UNK func_80076548(M2C_UNK);                     /* extern */\n"
         "void func_X(void *arg0, s32 arg1) {\n    func_80076548(0x12);\n    rcos(arg1);\n"
         "    if (arg1) { func_80001000(); }\n}\n")
    got = dict(levers.lever_args(t, "func_X"))
    v = got["args func_80076548 +arg0 proto"]
    assert v.startswith("M2C_UNK func_80076548(void *, M2C_UNK);                     /* extern */\n")
    assert "    func_80076548(arg0, 0x12);\n" in v
    assert got["args func_80076548 +arg0 noproto"].startswith("M2C_UNK func_80076548();")
    assert "func_80076548(0x12, arg1);" in got["args func_80076548 arg1+ proto"]
    assert "func_80076548(arg0, arg1, 0x12);" in got["args func_80076548 +arg0,arg1 proto"]
    # no local prototype: a func_<ADDR> callee goes through an unprototyped cast; rcos (headers) is left alone
    assert "((M2C_UNK (*)())func_80001000)(arg0)" in got["args func_80001000 arg0+ cast"]
    assert not any(" rcos " in k for k in got)
    assert levers.lever_args("void func_X(void) { func_80001000(1); }\n", "func_X") == []


def test_diff_facts():
    retail = ["lui v0,0xa0", "div zero,v0,v1", "addiu a0,s0,-16", "li a0,4", "sll v0,v0,0x2",
              "lui a1,0x8017", "slt v0,a0,v1", "lw v1,8(v0)"]
    ours = ["lui v0,0x140", "div zero,v0,v1", "addiu a0,s0,16", "li a0,6", "sll v0,v0,0x3",
            "lui a1,0x8018", "slt v0,v1,a0", "lw v1,8(v0)"]
    f = levers.diff_facts(retail, ours)
    assert f["pairs"] == [(0x1400000, 0xA00000), (16, -16), (6, 4), (3, 2), (8, 4)]   # no address lui
    assert f["cmp"]
    assert levers.diff_facts(retail, retail) == {"pairs": [], "cmp": False}
    # different sizes: only the equal-length replace blocks pair up
    f = levers.diff_facts(["li a0,4", "jal 0x80012345", "nop"], ["li a0,6", "jal 0x80012345", "nop", "nop"])
    assert (6, 4) in f["pairs"]


def test_lever_imm_and_cmp():
    t = ("void func_X(s32 *a) {\n    t = a[0] + 0x10;\n    if (t >= 6) {\n        f(6);\n    }\n"
         "    k = 0x1400000 / t;\n    m = a[1] - 5;\n}\n")
    facts = {"pairs": [(16, -16), (6, 4), (0x1400000, 0xA00000), (-5, -3)], "cmp": True}
    got = dict(levers.lever_imm(t, "func_X", facts))
    assert "    t = a[0] - 0x10;" in got["imm + 0x10->- 0x10"]
    assert "k = 0xA00000 / t;" in got["imm 0x1400000->0xA00000"]
    assert "m = a[1] - 3;" in got["imm - 5->- 3"]
    assert "if (t >= 4)" in got["imm 6->4"] or "f(4);" in got["imm 6->4"]
    off = dict(levers.lever_imm("void func_X(void) {\n    x = y < 7;\n}\n", "func_X", facts))
    assert list(off) == ["imm 7->5"]                               # `< 7` is `slti 6`: keep the +1
    cmp = dict(levers.lever_cmp(t, "func_X", facts))
    assert "if (t <= 6)" in cmp["cmp >=-><="] and len(cmp) == 3
    assert levers.lever_cmp(t, "func_X", {"pairs": [], "cmp": False}) == []
    assert levers.variants(t, "func_X", "imm,cmp") == []          # no facts, no diff levers


def test_cfile_candidate_specs(tmp_path):
    c = permute.cfile_candidate("ovl201:/c/x/func_80064C18.near-miss.c")
    assert (c["unit"], c["vaddr"], c["variant"]) == ("ovl201", "0x80064C18", "file")
    if os.name == "nt":
        assert c["cfile"].replace("\\", "/").startswith("C:/x/")
    c = permute.cfile_candidate(os.path.join("w", "exe_80012A78_ctx-knr.c"))
    assert (c["unit"], c["vaddr"], c["variant"]) == ("exe", "0x80012A78", "ctx-knr")
    c = permute.cfile_candidate(os.path.join("re", "match", "ovl202", "func_8004EADC.c"))
    assert (c["unit"], c["vaddr"]) == ("ovl202", "0x8004EADC")


def test_oracle_on_a_matched_file(tools):
    """permute.oracle() through sweep.oracle on a manifest row: clean MATCH, 3-tuple."""
    cpath = os.path.join(REPO, "re", "match", "ovl201", "func_80086FD4.c")
    if not os.path.isfile(cpath):
        pytest.skip("re/match/ovl201/func_80086FD4.c not in this checkout")
    try:
        import units
        units.resolve("ovl201")
    except Exception as e:      # noqa: BLE001 - no carved overlay here
        pytest.skip(str(e))
    c = {"unit": "ovl201", "vaddr": "0x80086FD4"}
    out = permute.oracle(c, cpath, {"size": 40, "rodata": None})
    assert len(out) == 3 and out[0] is True, out
