#include "ac1/ac1.h"
#include "ac1/i64.h"

/* -x for a soft-float double; 0 stays +0. */
double func_800B6D4C(double x) {
    DW u;

    u.d = x;
    if (u.w.hi != 0 || u.w.lo != 0) {
        u.w.hi ^= 0x80000000;
        return u.d;
    }
    return x;
}
