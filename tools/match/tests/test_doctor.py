"""tools/loop/doctor.py and regtie.py: the pieces that classify a near miss and read a tie.

    python -m pytest tools/match/tests/test_doctor.py

- word_kind() tells a register-only, an immediate-only and an opcode difference apart;
- skeleton() keeps only the control-flow opcodes;
- regtie's dump readers: priorities from .lreg lines, the allocation order from .greg,
  pseudo -> hard register by insn uid, register variables from the COFF info;
- regtie's split/reuse lever renames a reused variable from a later assignment on.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import doctor  # noqa: E402
import regtie  # noqa: E402


def test_word_kind():
    assert doctor.word_kind("addiu v1,v1,-1", "addiu a2,a2,-1") == "reg"
    assert doctor.word_kind("li a0,4", "li a0,6") == "imm"
    assert doctor.word_kind("lw v0,16(s0)", "lw v0,20(s0)") == "imm"
    assert doctor.word_kind("lh v0,0(a0)", "lhu v0,0(a0)") == "op"
    assert doctor.word_kind("move a0,s0", "move a0,s0") == "same"


def test_skeleton_keeps_control_flow():
    rows = ["addiu sp,sp,-24", "beqz a0,0x80001000", "nop", "jal 0x80002000", "move a0,s0", "jr ra", "nop"]
    assert doctor.skeleton(rows) == ["beqz", "jal", "jr"]


LREG = """
Register 72 used 9 times across 11 insns; GR_REGS or none.

Register 73 used 11 times across 9 insns; GR_REGS or none.

Register 74 used 4 times across 3 insns in block 2; GR_REGS or none.

(insn 10 8 12 (set (reg/v:SI 72)
        (reg:SI 4 a0)) -1 (nil)
    (nil))

(insn 12 10 14 (set (reg/v:SI 73)
        (const_int 0)) -1 (nil)
    (nil))
"""
GREG = """
;; 2 regs to allocate: 73 72
(insn 10 8 12 (set (reg/v:SI 3 v1)
        (reg:SI 4 a0)) -1 (nil)
    (nil))

(insn 12 10 14 (set (reg/v:SI 6 a2)
        (const_int 0)) -1 (nil)
    (nil))
"""
ASM = """\t.def\tfunc_80001000;\t.val\tfunc_80001000;\t.scl\t2;\t.type\t0x24;\t.endef
\t.def\tk;\t.val\t3;\t.scl\t4;\t.type\t0x4;\t.endef
\t.def\tk2;\t.val\t6;\t.scl\t4;\t.type\t0x4;\t.endef
\t.def\t.ef;\t.val\t.;\t.scl\t101;\t.line\t9;\t.endef
"""


def test_dump_readers():
    ps = regtie.pseudos(LREG)
    assert ps[72]["refs"] == 9 and ps[72]["live"] == 11 and not ps[72]["local"]
    assert ps[74]["local"]
    assert abs(ps[73]["prio"] - 3 * 11 / 9) < 1e-9          # floor_log2(11) = 3
    assert regtie.alloc_order(GREG) == [73, 72]
    hard = regtie.pseudo_regs(LREG, GREG, 72)
    assert hard[72] == (3, True) and hard[73] == (6, True)
    assert regtie.coff_vars(ASM, "func_80001000") == {"k": 3, "k2": 6}


def test_split_and_reuse():
    text = ("void func_80001000(s32 n) {\n    s32 k;\n    s32 k2;\n"
            "    k2 = n;\n    do { f(k2); } while (--k2 != 0);\n"
            "    k = n;\n    do { g(k); } while (--k != 0);\n"
            "    k = n * 3;\n    h(k);\n}\n")
    out = dict(regtie.lever_split(text, "func_80001000", ["k", "k2"], [("k", "k2")]))
    reuse = [t for d, t in out.items() if d.startswith("reuse k2 for k from k = n;")]
    assert reuse and "g(k2)" in reuse[0] and "h(k2)" in reuse[0]
    split = [t for d, t in out.items() if d.startswith("split k at k = n * 3")]
    assert split and "s32 k_b;" in split[0] and "h(k_b)" in split[0] and "g(k)" in split[0]
