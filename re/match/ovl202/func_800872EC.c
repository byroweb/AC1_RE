#include "ac1/ac1.h"

extern s32 func_800871CC(s32 range);

/* func_80087208 for four channels */
void func_800872EC(u8 *r, u8 *g, u8 *b, u8 *a)
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
    *r = func_800871CC(range) + avg;
    *g = func_800871CC(range) + avg;
    *b = func_800871CC(range) + avg;
    *a = func_800871CC(range) + avg;
}
