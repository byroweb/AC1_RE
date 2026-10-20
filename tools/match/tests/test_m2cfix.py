"""Text-only tests for tools/loop/m2cfix.py (the sweep's draft fix-ups); no toolchain needed."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "loop"))
import m2cfix  # noqa: E402

TEMPLATE = """struct _m2c_stack_func_80001000 {
    /* 0x00 */ char pad0[0x10];
    /* 0x10 */ u16 sp10;                            /* inferred */
    /* 0x12 */ s8 sp12;                             /* inferred */
    /* 0x14 */ s32 sp14;                            /* inferred */
    /* 0x18 */ char pad18[8];
};                                                  /* size = 0x20 */

"""

DRAFT = """M2C_UNK func_80002000(u16 *);                      /* extern */

s32 func_80001000(void *arg0) {
    void **sp40;
    u16 sp10;                                       /* compiler-managed */
    s32 (*var_v0)(s32);
    s32 temp_v1;

    func_80002000(&sp10);
    temp_v1 = sp10 + sp12 + saved_reg_s0;
    return temp_v1 + sp14;
}
"""


def test_declared_names():
    lines = DRAFT.split("\n")
    names, after = m2cfix.declared_names(lines, 2)
    assert names == {"sp40": "void **", "sp10": "u16", "var_v0": "fnptr *", "temp_v1": "s32"}
    assert lines[after] == ""


def test_declare_locals_uses_template_types():
    out = m2cfix.declare_locals(TEMPLATE + DRAFT, "func_80001000")
    assert "struct _m2c_stack" not in out
    assert "    s8 sp12;" in out and "    s32 sp14;" in out and "    M2C_UNK saved_reg_s0;" in out
    assert out.count(" sp10;") == 1 and out.count("sp40;") == 1


def test_stack_frame_one_union():
    # addiu sp,sp,-0x20 ; sw ra,0x18(sp)
    blob = bytes.fromhex("e0ffbd27") + bytes.fromhex("1800bfaf")
    out = m2cfix.stack_frame(TEMPLATE + DRAFT, "func_80001000", blob=blob)
    assert "union { s8 b[0x8]; s32 w; } sp_frame;" in out
    assert "func_80002000(&(*(u16 *)(sp_frame.b + 0x0)));" in out
    assert "(*(s8 *)(sp_frame.b + 0x2))" in out and "(*(s32 *)(sp_frame.b + 0x4))" in out
    assert " sp10;" not in out and "void **sp40;" in out      # sp40 is above the frame


def test_cast_derefs():
    f = m2cfix.cast_derefs
    assert f("    temp_s1 = *(temp_s2 + *temp_s0);", {"temp_s0"}) == \
        "    temp_s1 = *(M2C_UNK *)(temp_s2 + *temp_s0);"
    assert f("    x = (u8) *(a + b);", set()) == "    x = (u8) *(M2C_UNK *)(a + b);"
    assert f("    x = a * (b);", set()) == "    x = a * (b);"
    assert f("    x = *(u16 *)p;", set()) == "    x = *(u16 *)p;"
    assert f("    x = *(a + *(void **)0x1F80001C);", set()) ==         "    x = *(M2C_UNK *)(a + *(void **)0x1F80001C);"
    assert f("    x = *(u8 * *)(b + 4);", set()) == "    x = *(u8 * *)(b + 4);"
    assert f("    *p = (*(M2C_UNK *)p & 0xFF) | (*q & 1);", {"p"}) == \
        "    *p = (*(M2C_UNK *)p & 0xFF) | (*(M2C_UNK *)q & 1);"


def test_cast_pointer_operands():
    f = m2cfix.cast_pointer_operands
    assert f("    x = (s32) (var_t1 - arg0);", {"var_t1", "arg0"}) == \
        "    x = (s32) (((s32) var_t1) - ((s32) arg0));"
    assert f("    x = 0x801F8000 - &D_80039CC0;", set()) == "    x = 0x801F8000 - ((s32) &D_80039CC0);"
    assert f("    x = a & b;", set()) == "    x = a & b;"
    assert f("    if (a && b) {", set()) == "    if (a && b) {"
    assert f("    p->q = p - 1;", {"p"}) == "    p->q = ((s32) p) - 1;"
    assert f("    p += q - *(u8 * *)(b + 4);", {"p", "q"}) == "    p += ((s32) q) - *(s32 *)(b + 4);"
    assert f("    x = *t * 0x19458;", {"t"}, {"t"}) == "    x = *(s32 *)t * 0x19458;"
    assert f("    v = (s32) ((s32) a) - g_ws >> 6;", {"a"}) == "    v = (s32) ((s32) a) - ((s32) g_ws) >> 6;"


def test_void_casts():
    m2cfix._VOID_FNS = {"proj_init"}
    try:
        out = m2cfix.void_casts("    x = proj_init(a);\n    proj_init(b);\n    y = *(void *)0x1F800004;")
    finally:
        m2cfix._VOID_FNS = None
    assert out == ("    x = ((M2C_UNK (*)())proj_init)(a);\n    proj_init(b);\n"
                   "    y = *(M2C_UNK *)0x1F800004;")


def test_map_errors(tmp_path):
    text = '#include "ac1/ac1.h"\n\nint g;\n\n/* c */\nvoid f(void) {\n    g = 1;\n}\n'
    fn_i = tmp_path / "fn.i"
    fn_i.write_text("typedef int s32;\nint h;\nint g;\nvoid f(void) {\n    g = 1;\n}\n")
    assert m2cfix.map_errors(text, str(fn_i), [(5, "boom"), (1, "header")]) == [(6, "boom")]


def test_repair_declares_and_casts():
    text = "s32 f(s32 arg0) {\n    s32 a;\n\n    a = *(arg0 + 4);\n    return sp10 + a;\n}\n"
    out = m2cfix.repair(text, "f", [(3, "invalid type argument of `unary *'"),
                                    (4, "`sp10' undeclared (first use this function)")])
    assert "    M2C_UNK sp10;" in out and "a = *(M2C_UNK *)(arg0 + 4);" in out
    assert m2cfix.repair(text, "f", [(3, "something else")]) is None
