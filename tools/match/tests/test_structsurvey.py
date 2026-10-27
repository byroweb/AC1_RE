"""structsurvey.py layout + binding detection, and ctx.py's opt-in header blocks."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

import ctx  # noqa: E402
import structsurvey as ss  # noqa: E402

SRC = """#include "ac1/ac1.h"
typedef struct Inner {
    s16 a;              /* +0x00 */
    u8 b;               /* +0x02 */
} Inner;
typedef struct {
    u32 used;           /* +0x00 */
    s16 d1, d2;         /* +0x04 */
    Inner in[2];        /* +0x08 */
    u8 pad10[0x18 - 0x10];
    void *p;            /* +0x18 */
    union { u8 b[2]; u16 h; } k; /* +0x1C */
    SVECTOR v;          /* +0x1E */
} Blk;
extern Blk D_801A2550;
extern Blk *D_80039D18;
#define SP ((Inner *)0x1F800000)
void f(void) { struct { int x; } local; }
"""


def unit(tmp_path):
    p = tmp_path / "func_80000000.c"
    p.write_text(SRC)
    return ss.Unit(str(p), "ovl201/func_80000000.c", ss.header_sizes(), {})


def test_layout(tmp_path):
    u = unit(tmp_path)
    lay = u.types["Blk"]
    offs = {f["name"]: (f["off"], f["size"]) for f in lay["fields"]}
    assert offs["d2"] == (6, 2)
    assert offs["in"] == (8, 8)           # Inner is 4 bytes (s16 + u8, aligned 2)
    assert offs["p"] == (0x18, 4)
    assert offs["k"] == (0x1C, 2)
    assert offs["v"] == (0x1E, 8)         # SVECTOR aligns to 2
    assert lay["size"] == 0x28 and lay["ok"]
    assert not any("mismatch" in f for f in lay["fields"])


def test_bindings(tmp_path):
    b = unit(tmp_path).bindings()
    assert (0x801A2550, "Blk", "extern") in b
    assert (0x80039D18, "Blk", "extern *") in b
    assert (0x1F800000, "Inner", "cast") in b


def test_header_block():
    src = "x\n%s\nold;\n%s\n#endif\n" % (ctx.BLOCK_BEGIN, ctx.BLOCK_END)
    rows = [dict(addr=0x801A2550, name="g_x", ctype="PadState", unit="201")]
    out = ctx.render_header_block(src, rows, "t.h")
    assert "old;" not in out
    assert "extern PadState g_x;" in out and out.endswith(ctx.BLOCK_END + "\n#endif\n")


def test_opt_in_rows_leave_symbols_h():
    base, ext = ctx.split_rows(ctx.load_syms())
    assert "frontend.h" in ext and any(r["name"] == "g_fe_pad" for r in ext["frontend.h"])
    assert all(r["name"] != "g_fe_pad" for r in base)
    assert "g_fe_pad" not in ctx.render_symbols_h(ctx.load_syms())
