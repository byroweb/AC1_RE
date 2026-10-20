"""ctx.py names a switch's jump table so m2c can build the switch.

    python -m pytest tools/match/tests/test_ctx_jtbl.py

Pure text tests: no toolchain, disc image or m2c needed.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import ctx  # noqa: E402

# ovl202 func_80081318's switch head (objdump2m2c output), cut down
SWITCH = """glabel func_80081318
    lbu $v1, 100($s0)
    nop
    sltiu $v0, $v1, 3
    beqz $v0, .L80081350
    sll $v0, $v1, 0x2
    lui $at, 0x8005
    addu $at, $at, $v0
    lw $v0, -17504($at)
    nop
    jr $v0
    nop
    li $v0, 2
    b .L80081354
    nop
.L80081350:
    li $v0, 1
.L80081354:
    jr $ra
    nop
"""


class NoSyms:
    def func(self, a):
        return None

    def lookup(self, a):
        return "D_%08X" % a


def test_indexed_table_load_is_named():
    out = ctx.symbolise(SWITCH, NoSyms())
    assert "lui $at, %hi(jtbl_8004BBA0)" in out
    assert "lw $v0, %lo(jtbl_8004BBA0)($at)" in out


def test_indexed_load_not_feeding_jr_stays_raw():
    asm = SWITCH.replace("    jr $v0\n", "    move $a0, $v0\n", 1)
    out = ctx.symbolise(asm, NoSyms())
    assert "%hi(" not in out and "-17504($at)" in out


def test_tables_and_case_labels():
    sym = ctx.symbolise(SWITCH, NoSyms())
    # three cases: 0x80081344 (li $v0, 2), 0x80081350 (default), 0x80081344 again;
    # then a word outside the function ends the table
    mem = {0x8004BBA0: 0x80081344, 0x8004BBA4: 0x80081350, 0x8004BBA8: 0x80081344,
           0x8004BBAC: 0x80012345}
    out = ctx.add_jump_tables(sym, 0x80081318, 0x44, mem.get)
    lines = out.splitlines()
    assert lines[lines.index("    li $v0, 2") - 1] == ".L80081344:"
    assert out.count(".L80081350:") == 1
    tail = out[out.index(".section .rodata"):].splitlines()
    assert tail == [".section .rodata", "jtbl_8004BBA0:",
                    ".word .L80081344", ".word .L80081350", ".word .L80081344"]


def test_table_stops_at_next_known_table():
    sym = ctx.symbolise(SWITCH, NoSyms())
    mem = {0x8004BBA0: 0x80081344, 0x8004BBA4: 0x80081344}
    out = ctx.add_jump_tables(sym, 0x80081318, 0x44, mem.get, known=[0x8004BBA4])
    assert out.rstrip().endswith("jtbl_8004BBA0:\n.word .L80081344")


def test_no_table_no_change():
    asm = "glabel f\n    jr $ra\n    nop\n"
    assert ctx.add_jump_tables(asm, 0x80010000, 8, lambda a: None) == asm


# ovl202 func_80053190: the table base is hoisted out of the loop into $s0, and
# functions.tsv has no jt_addrs for it; the retail word at the address decides
HOISTED = """glabel func_80053190
    lui $s0, 0x8005
    addiu $s0, $s0, -20312
.L80053198:
    sltiu $v0, $v1, 2
    beqz $v0, .L800531b8
    sll $v0, $v1, 0x2
    addu $v0, $v0, $s0
    lw $v0, 0($v0)
    nop
    jr $v0
    nop
    b .L80053198
    nop
.L800531b8:
    jr $ra
    nop
"""


def test_hoisted_base_found_from_retail_word():
    mem = {0x8004B0A8: 0x800531B0, 0x8004B0AC: 0x800531B8}
    jt = ctx.JumpTables([], HOISTED, 0x80053190, 0x30, mem.get)
    out = ctx.symbolise(HOISTED, NoSyms(), jt)
    assert "addiu $s0, $s0, %lo(jtbl_8004B0A8)" in out
    out = ctx.add_jump_tables(out, 0x80053190, 0x30, mem.get)
    assert out.rstrip().endswith("jtbl_8004B0A8:\n.word .L800531b0\n.word .L800531b8")


def test_no_register_jr_no_scan():
    asm = HOISTED.replace("    jr $v0\n", "    move $a0, $v0\n", 1)
    mem = {0x8004B0A8: 0x800531B0}
    assert 0x8004B0A8 not in ctx.JumpTables([], asm, 0x80053190, 0x30, mem.get)
