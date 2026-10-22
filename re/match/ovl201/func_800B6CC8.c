#include "ac1/ac1.h"
#include "ac1/i64.h"

extern double func_800B59F4(double, double);
extern double func_800B6D4C(double);

/* a - b for soft-float doubles: a + (-b), with the zero operands shortcut. */
double func_800B6CC8(double a, double b) {
    DW ua;
    DW ub;

    ua.d = a;
    ub.d = b;
    if ((ub.w.hi & 0x7FFFFFFF) == 0 && ub.w.lo == 0) {
        return a;
    }
    if ((ua.w.hi & 0x7FFFFFFF) == 0 && ua.w.lo == 0) {
        return func_800B6D4C(ub.d);
    }
    ub.w.hi ^= 0x80000000;
    return func_800B59F4(a, ub.d);
}
