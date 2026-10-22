/* fdat014 0x801C63DC  size 268  zero-agent sweep (tools/loop/sweep.py): neighbour draft of ovl202 0x800872EC (ovl202/func_800872EC.c, similarity 1.000), 2026-10-22 */
#include "ac1/ac1.h"

extern s32 func_801C62BC(s32 range);

/* func_80087208 for four channels */
void func_801C63DC(u8 *r, u8 *g, u8 *b, u8 *a)
{
    s32 cr;
    s32 cg;
    s32 cb;
    s32 ca;
    s32 avg;
    s32 hi;
    s32 lo;
    s32 range;

    cr = *r;
    cg = *g;
    cb = *b;
    ca = *a;
    avg = (cr + cg + cb + ca) >> 2;
    if (cg < cr) {
        hi = cr;
        lo = cg;
    } else {
        hi = cg;
        lo = cr;
    }
    if (hi < cb) {
        hi = cb;
    }
    if (hi < ca) {
        hi = ca;
    }
    if (cb < lo) {
        lo = cb;
    }
    if (ca < lo) {
        lo = ca;
    }
    range = hi - lo;
    *r = func_801C62BC(range) + avg;
    *g = func_801C62BC(range) + avg;
    *b = func_801C62BC(range) + avg;
    *a = func_801C62BC(range) + avg;
}
