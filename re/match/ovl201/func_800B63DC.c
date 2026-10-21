#include "ac1/ac1.h"
#include "ac1/i64.h"

extern U64 func_800B6214(s32 dir, U64 v, s32 n);

/* (double)x for a soft-float double. */
double func_800B63DC(s32 x) {
    DW u;
    U64 m;
    u32 sign;
    s32 e;

    sign = 0;
    e = 1053;
    if (x == 0) {
        u.w.lo = 0;
        u.w.hi = 0;
        return u.d;
    }
    if (x < 0) {
        sign = 0x80000000;
        x = -x;
    }
    while (x < 0x1000000) {
        x <<= 4;
        e -= 4;
    }
    while (x < 0x40000000) {
        x <<= 1;
        e--;
    }
    m.lo = 0;
    m.hi = x;
    m = func_800B6214(1, m, 10);
    m.hi &= ~0x100000;
    u.w.hi = m.hi | sign;
    u.w.hi |= e << 20;
    u.w.lo = m.lo;
    return u.d;
}
