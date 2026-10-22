#include "ac1/ac1.h"
#include "ac1/i64.h"

extern I64 func_800B5D74(I64 a, I64 b);

/* -a for a 64-bit value: ~a + 1. */
I64 func_800B6850(I64 a) {
    I64 one;

    one.hi = 0;
    one.lo = 1;
    a.hi = ~a.hi;
    a.lo = ~a.lo;
    a = func_800B5D74(a, one);
    return a;
}
