"""sweep.py's neighbour variant: slot alignment and the constant fix-up.

    python -m pytest tools/match/tests/test_sweep_neighbour.py

Pure functions only (no image, no toolchain).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(HERE)), "loop"))
import sweep  # noqa: E402
import twins  # noqa: E402

LUI_V0 = 0x3C020000          # lui v0,IMM
LW_V0 = 0x8C420000           # lw v0,IMM(v0)
JAL = 0x0C000000
NOP = 0
ADDIU_A0 = 0x24840000        # addiu a0,a0,IMM


def words(va, data_hi, data_lo, callee, extra=0):
    w = [LUI_V0 | data_hi, LW_V0 | data_lo, JAL | (callee >> 2 & 0x03FFFFFF), NOP]
    return w[:2] + [ADDIU_A0 | k for k in range(extra)] + w[2:]


def test_align_slots_moves_names_across_an_insertion():
    # neighbour at 0x80050000 reads D_80060010 and calls func_80051000; the body has two
    # extra instructions in between and reads/calls the same objects 0x100 higher
    na, nw = 0x80050000, words(0x80050000, 0x8006, 0x0010, 0x80051000)
    ba, bw = 0x80070000, words(0x80070000, 0x8006, 0x0110, 0x80051100, extra=2)
    nm, ns = twins.mask_function(nw, na)
    bm, bs = twins.mask_function(bw, ba)
    a, b = sweep.align_slots(nm, ns, bm, bs)
    assert [s[1] for s in a] == ["HI", "LO", "J"] == [s[1] for s in b]
    assert twins.map_addr(0x80060010, a, b, "exe") == (0x80060110, None)
    assert twins.map_addr(0x80060000, a, b, "exe") == (0x80060100, None)   # D_ base, field +0x10
    assert twins.map_addr(0x80051000, a, b, "exe") == (0x80051100, None)


def test_imm_fixup_loop_bound():
    text = "/* hdr 0x80066F68 */\nu32 f(u8 *p) { u32 i = 0; do { i++; } while (i < 0xBE4); return i; }\n"
    out = sweep.imm_fixup(text, [("!", 0x800B4384, "sltiu v0,v1,14562", "sltiu v0,v1,3044")])
    assert "while (i < 0x38E2)" in out and out.startswith("/* hdr 0x80066F68 */\n")


def test_imm_fixup_compare_and_negative_forms():
    text = "/**/\nx = 0x40U; if (y > 0x40) z = a - 5;\n"
    diffs = [("!", 0, "slti v0,v0,-63", "slti v0,v0,65"),       # y > 0x40: slti 0x41
             ("!", 4, "addiu v0,v0,-7", "addiu v0,v0,-5")]      # a - 5: addiu -5
    out = sweep.imm_fixup(text, diffs)
    assert out == "/**/\nx = (-0x40U); if (y > (-0x40)) z = a - 7;\n"


def test_imm_fixup_refuses_other_differences():
    text = "/**/\nf(0x10);\n"
    assert sweep.imm_fixup(text, [("!", 0, "lw a0,16(a1)", "lw a0,16(a2)")]) is None
    assert sweep.imm_fixup(text, [("!", 0, "jal 0x80010000", "jal 0x80020000")]) is None
    assert sweep.imm_fixup(text, [("!", 0, "li a0,32", "nop")]) is None
