/* fdat014 0x801C62F8  size 228  zero-agent sweep (tools/loop/sweep.py): neighbour draft of ovl202 0x80087208 (ovl202/func_80087208.c, similarity 1.000), 2026-10-22 */
#include "ac1/ac1.h"

extern s32 func_801C62BC(s32 range);

/* jitter an RGB triple around its grey level by up to its own spread */
void func_801C62F8(u8 *r, u8 *g, u8 *b)
{
    s32 cr;
    s32 cg;
    s32 cb;
    s32 avg;
    s32 hi;
    s32 lo;
    s32 range;

    cr = *r;
    cg = *g;
    cb = *b;
    avg = (cr + cg + cb) / 3;
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
    if (cb < lo) {
        lo = cb;
    }
    range = hi - lo;
    *r = func_801C62BC(range) + avg;
    *g = func_801C62BC(range) + avg;
    *b = func_801C62BC(range) + avg;
}
