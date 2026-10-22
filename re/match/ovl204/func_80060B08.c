/* ovl204 0x80060B08  size 332  local LLM (tools/loop/golem.py tweak, qwen2.5-coder-7b-instruct), 2026-10-21 */
#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(r0))
#define gte_rtv0() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ("swc2 $25, 0(%0)\n\tswc2 $26, 4(%0)\n\tswc2 $27, 8(%0)" : : "r"(r0) : "memory")

#define SCR_V ((VECTOR *)0x1F800300)

extern s32 func_80060AC4(s32 x, s32 y);

/* is a box of half-width w and height h around v (rotated into view space)
 * on screen and nearer than far?  func_80060AC4 returns the outcode of one
 * screen point; the box is visible unless all four corners share a bit */
s32 func_80060B08(s32 far, SVECTOR *v, s32 w, s32 h)
{
    VECTOR *o;
    s32 k;
    s32 x;
    s32 y;
    s32 r;
    s32 hh;
    s32 x0;
    s32 x1;
    s32 c;
    s32 y2;

    o = SCR_V;
    gte_ldv0(v);
    gte_rtv0();
    gte_stlvnl(o);
    SCR_V->vz += w;
    if (SCR_V->vz > 0 && far >= SCR_V->vz) {
        k = 0xa00000 / SCR_V->vz;
        x = (o->vx * k) >> 16;
        y = (SCR_V->vy * k) >> 16;
        r = (w * k) >> 16;
        hh = (h * k) >> 16;
        x0 = x - r;
        c = func_80060AC4(x0, y);
        x1 = x + r;
        c &= func_80060AC4(x1, y);
        y2 = y - hh;
        c &= func_80060AC4(x0, y2);
        c &= func_80060AC4(x1, y2);
        return c == 0;
    }
    return 0;
}
