#include "ac1/ac1.h"
#include "ac1/i64.h"

extern I64 func_800B5E24(s32 dir, I64 v, s32 n);
extern s32 func_800B6D84(s32 err, s32 arg);

/* (int)x for a soft-float double; out of range raises ERANGE and saturates. */
s32 func_800B62DC(double x) {
    DW u;
    I64 m;
    s32 e;
    s32 sh;
    s32 v;

    u.d = x;
    if ((u.w.hi & 0x7FFFFFFF) == 0 && u.w.lo == 0) {
        return u.w.hi & 0x80000000;
    }
    e = ((s32)u.w.hi >> 20) & 0x7FF;
    sh = e - 1053;
    if (sh > 0) {
        func_800B6D84(34, 17);
        return (s32)u.w.hi >= 0 ? 0x7FFFFFFF : 0x80000000;
    }
    m.hi = (u.w.hi & 0xFFFFF) | 0x100000;
    m.lo = u.w.lo;
    m = func_800B5E24(0, m, 10);
    v = m.hi;
    if ((u32)(e - 1022) < 32 && v != 0) {
        v >>= -sh;
    } else {
        return 0;
    }
    return (s32)u.w.hi < 0 ? -v : v;
}
