/* MATCHED (wave4 a6): separate non-call-crossing temp m = sh + 1 in arm2 and the shift call in each arm, cross-jumped tail. */
#include "ac1/ac1.h"
#include "ac1/i64.h"

extern U64 func_800B5D74(U64 a, U64 b);
extern U64 func_800B5E24(s32 dir, U64 v, s32 n);
extern s32 func_800B61D0(U64 a, U64 b);
extern U64 func_800B6214(s32 dir, U64 v, s32 n);
extern U64 func_800B6850(U64 a);
extern s32 func_800B6D84(s32 err, s32 arg);

/* a / b for soft-float doubles: restoring division, one quotient bit per step. */
double func_800B5EEC(double a, double b)
{
    DW ua;
    DW ub;
    U64 q;
    U64 bit;
    U64 rnd;
    U64 ma;
    U64 mb;
    DW r;
    u32 sign;
    s32 e;
    s32 sh;
    s32 n;
    s32 m;

    ua.d = a;
    ub.d = b;
    e = (((s32)ua.w.hi >> 20) & 0x7FF) - (((s32)ub.w.hi >> 20) & 0x7FF) + 1022;
    sign = (ua.w.hi & 0x80000000) ^ (ub.w.hi & 0x80000000);
    if ((ub.w.hi & 0x7FFFFFFF) == 0 && ub.w.lo == 0) {
        if (sign != 0) {
            r.w.hi = 0xFFFFFFFF;
        } else {
            r.w.hi = 0x7FFFFFFF;
        }
        r.w.lo = 0xFFFFFFFF;
        return r.d;
    }
    if ((ua.w.hi & 0x7FFFFFFF) == 0 && ua.w.lo == 0) {
        r.w.hi = sign;
        r.w.lo = 0;
        return r.d;
    }
    ma.hi = (ua.w.hi & 0xFFFFF) | 0x100000;
    ma.lo = ua.w.lo;
    mb.hi = (ub.w.hi & 0xFFFFF) | 0x100000;
    mb.lo = ub.w.lo;
    if (func_800B61D0(ma, mb) < 0) {
        ma = func_800B5E24(0, ma, 1);
        e--;
    }
    bit.hi = 0x200000;
    bit.lo = 0;
    q.lo = 0;
    q.hi = 0;
    do {
        if (func_800B61D0(ma, mb) >= 0) {
            q.hi |= bit.hi;
            q.lo |= bit.lo;
            ma = func_800B5D74(ma, func_800B6850(mb));
        }
        ma = func_800B5E24(0, ma, 1);
        bit = func_800B6214(1, bit, 1);
    } while (bit.hi != 0 || bit.lo != 0);
    if (e >= 0) {
        n = 1;
        rnd.hi = 0;
        rnd.lo = 1;
        q = func_800B5D74(q, rnd);
        e++;
        q = func_800B6214(1, q, n);
    } else {
        sh = -e;
        rnd.hi = 0;
        rnd.lo = 1 << sh;
        q = func_800B5D74(q, rnd);
        e = 0;
        m = sh + 1;
        q = func_800B6214(1, q, m);
    }
    q.hi &= ~0x100000;
    if (e >= 2047) {
        func_800B6D84(34, 15);
        r.w.hi = sign != 0 ? 0xFFF00000 : 0x7FF00000;
        r.w.lo = 0;
    } else {
        r.w.hi = sign | (e << 20) | q.hi;
        r.w.lo = q.lo;
    }
    return r.d;
}
