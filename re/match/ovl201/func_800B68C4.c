#include "ac1/ac1.h"
#include "ac1/i64.h"

extern U64 func_800B6214(s32 dir, U64 v, s32 n);
extern U64 func_800B5D74(U64 a, U64 b);
extern U64 func_800B6BD8(u32 a, u32 b);
extern s32 func_800B6D84(s32 err, s32 arg);

/* a * b for soft-float doubles: 21-bit limbs, rounded to 53 bits. */
double func_800B68C4(double a, double b) {
    DW ua;
    DW ub;
    U64 p;
    U64 rnd;
    U64 ma;
    U64 mb;
    U64 t;
    U64 t2;
    DW r;
    u32 sign;
    s32 e;
    s32 exp;
    u32 ah;
    u32 bh;
    u32 al;
    u32 bl;
    u32 msb;

    ua.d = a;
    ub.d = b;
    sign = (ua.w.hi & 0x80000000) ^ (ub.w.hi & 0x80000000);
    if (((ua.w.hi & 0x7FFFFFFF) == 0 && ua.w.lo == 0) || ((ub.w.hi & 0x7FFFFFFF) == 0 && ub.w.lo == 0)) {
        r.w.hi = sign;
        r.w.lo = 0;
        return r.d;
    }
    ma.hi = (ua.w.hi & 0xFFFFF) | 0x100000;
    mb.hi = (ub.w.hi & 0xFFFFF) | 0x100000;
    e = (((s32)ua.w.hi >> 20) & 0x7FF) + (((s32)ub.w.hi >> 20) & 0x7FF);
    ma.lo = ua.w.lo;
    mb.lo = ub.w.lo;
    t = func_800B6214(1, ma, 21);
    ah = t.lo;
    t = func_800B6214(1, mb, 21);
    bh = t.lo;
    al = ma.lo & 0x1FFFFF;
    bl = mb.lo & 0x1FFFFF;
    p = func_800B6BD8(ah, bh);
    t2 = func_800B6BD8(al, bh);
    t = func_800B6214(1, t2, 21);
    p = func_800B5D74(p, t);
    t2 = func_800B6BD8(bl, ah);
    t = func_800B6214(1, t2, 21);
    exp = e - 1022;
    p = func_800B5D74(p, t);
    msb = 0x80000000;
    if (p.hi & msb) {
        rnd.hi = 0;
        rnd.lo = 1024;
        p = func_800B5D74(p, rnd);
        p = func_800B6214(1, p, 11);
    } else {
        rnd.hi = 0;
        rnd.lo = 512;
        p = func_800B5D74(p, rnd);
        if (p.hi & msb) {
            p = func_800B6214(1, p, 11);
        } else {
            p = func_800B6214(1, p, 10);
            exp = e - 1023;
        }
    }
    p.hi &= ~0x100000;
    if (exp >= 2047) {
        func_800B6D84(34, 13);
        r.w.hi = sign != 0 ? 0xFFF00000 : 0x7FF00000;
        r.w.lo = 0;
    } else {
        r.w.hi = sign | (exp << 20) | p.hi;
        r.w.lo = p.lo;
    }
    return r.d;
}
