#include "ac1/ac1.h"
#include "ac1/i64.h"

extern U64 func_800B5E24(s32 dir, U64 v, s32 n);
extern U64 func_800B5D74(U64 a, U64 b);
extern U64 func_800B6850(U64 a);
extern s32 func_800B6D84(s32 err, s32 arg);

/* a + b for soft-float doubles: two's complement mantissas with 9 guard bits. */
double func_800B59F4(double a, double b) {
    DW ua;
    DW ub;
    U64 ma;
    U64 mb;
    U64 rnd;
    DW z;
    u32 sign;
    u32 msb;
    s32 ea;
    s32 eb;

    z.w.lo = 0;
    z.w.hi = 0;
    sign = 0;
    ua.d = a;
    ub.d = b;
    if ((ua.w.hi & 0x7FFFFFFF) == 0 && ua.w.lo == 0) {
        return b;
    }
    if ((ub.w.hi & 0x7FFFFFFF) == 0 && ub.w.lo == 0) {
        return ua.d;
    }
    ea = ((s32)ua.w.hi >> 20) & 0x7FF;
    eb = ((s32)ub.w.hi >> 20) & 0x7FF;
    if (eb + 54 < ea) {
        return ua.d;
    }
    if (ea + 54 < eb) {
        return ub.d;
    }
    ma.hi = (ua.w.hi & 0xFFFFF) | 0x100000;
    ma.lo = ua.w.lo;
    mb.hi = (ub.w.hi & 0xFFFFF) | 0x100000;
    mb.lo = ub.w.lo;
    msb = 0x80000000;
    if (ua.w.hi & msb) {
        ma = func_800B6850(ma);
    }
    if (ub.w.hi & msb) {
        mb = func_800B6850(mb);
    }
    ma = func_800B5E24(0, ma, 9);
    mb = func_800B5E24(0, mb, 9);
    if (eb < ea) {
        mb = func_800B5E24(1, mb, ea - eb);
    } else {
        ma = func_800B5E24(1, ma, eb - ea);
        ea = eb;
    }
    ma = func_800B5D74(ma, mb);
    if ((s32)ma.hi < 0) {
        ma = func_800B6850(ma);
        sign = 0x80000000;
    } else if (ma.hi == 0 && ma.lo == 0) {
        return z.d;
    }
    while ((ma.hi & 0xE0000000) == 0) {
        ma = func_800B5E24(0, ma, 1);
        ea--;
    }
    if (ma.hi & 0x40000000) {
        ma = func_800B5E24(1, ma, 1);
        ea++;
    }
    rnd.hi = 0;
    rnd.lo = (ma.lo & 0x200) ? 256 : 255;
    ma = func_800B5D74(ma, rnd);
    if (ma.hi & 0x40000000) {
        ma = func_800B5E24(1, ma, 1);
        ea++;
    }
    ma = func_800B5E24(1, ma, 9);
    ma.hi &= ~0x100000;
    if (ea >= 2047) {
        func_800B6D84(34, 11);
        ua.w.hi = sign != 0 ? 0xFFF00000 : 0x7FF00000;
        ua.w.lo = 0;
    } else {
        ua.w.hi = sign | (ea << 20) | ma.hi;
        ua.w.lo = ma.lo;
    }
    return ua.d;
}
