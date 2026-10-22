#include "ac1/ac1.h"
#include "ac1/i64.h"

/* soft-float double compare, negated form (-1 or 0). */
/* the sign tests, written as branches (jump2 then forms slt / sltiu) */
#define RET_NEG { if (ua.w.hi & 0x80000000) return -1; return 0; }
#define RET_POS { if (ua.w.hi & 0x80000000) return 0; return -1; }

s32 func_800B64E0(double a, double b) {
    DW ua;
    DW ub;
    u32 sa;
    u32 sb;
    s32 ea;
    s32 eb;
    s32 ma;
    s32 mb;

    ua.d = a;
    ub.d = b;
    if (ua.w.hi == ub.w.hi && ua.w.lo == ub.w.lo) {
        return 0;
    }
    if ((ua.w.hi & 0x7FFFFFFF) == 0 && ua.w.lo == 0 && (ub.w.hi & 0x7FFFFFFF) == 0 && ub.w.lo == 0) {
        return 0;
    }
    sa = ua.w.hi & 0x80000000;
    sb = ub.w.hi & 0x80000000;
    if (sa != sb) {
        return -(sa != 0);
    }
    ea = ((s32)ua.w.hi >> 20) & 0x7FF;
    eb = ((s32)ub.w.hi >> 20) & 0x7FF;
    if (eb < ea) {
        goto larger;
    }
    if (ea != eb) {
        goto smaller;
    }
    ma = (ua.w.hi & 0xFFFFF) | 0x100000;
    mb = (ub.w.hi & 0xFFFFF) | 0x100000;
    if (mb < ma) {
        goto larger;
    }
    if (ma != mb) {
        goto smaller;
    }
    if (ua.w.lo < ub.w.lo) {
        goto smaller;
    }
larger:
    RET_NEG
smaller:
    RET_POS
}
