"""tools/loop/sweep.py opkey(): the opcode-only key behind the neighbour-op backup.

    python -m pytest tools/match/tests/test_neighbour_op.py

- registers and immediates do not change the key; the operation does;
- nop, SPECIAL functs, REGIMM branches and GTE commands each get keys of their own.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import sweep  # noqa: E402


def test_registers_and_immediates_ignored():
    assert sweep.opkey(0x24420004) == sweep.opkey(0x2484FFFC)     # addiu v0,v0,4 / addiu a0,a0,-4
    assert sweep.opkey(0x8C820000) == sweep.opkey(0x8FBF0014)     # lw v0,0(a0) / lw ra,20(sp)
    assert sweep.opkey(0x00431021) == sweep.opkey(0x00A42821)     # addu v0,v0,v1 / addu a1,a1,a0


def test_operations_kept_apart():
    assert sweep.opkey(0x00000000) != sweep.opkey(0x00000025)     # nop / or zero,zero,zero
    assert sweep.opkey(0x00431021) != sweep.opkey(0x00431023)     # addu / subu (SPECIAL funct)
    assert sweep.opkey(0x04410002) != sweep.opkey(0x04400002)     # bgez / bltz (REGIMM rt)
    assert sweep.opkey(0x8C820000) != sweep.opkey(0x84820000)     # lw / lh


def test_gte_commands_distinct():
    rtps, nclip = 0x4A180001, 0x4B400006
    mfc2 = 0x48026000                                             # mfc2 v0,$12
    assert len({sweep.opkey(rtps), sweep.opkey(nclip), sweep.opkey(mfc2)}) == 3
