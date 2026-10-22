#include "ac1/ac1.h"
#include "ac1/i64.h"

/* Unsigned three-way compare: 1, -1 or 0. */
s32 func_800B61D0(U64 a, U64 b) {
    if (a.hi > b.hi) {
        return 1;
    }
    if (a.hi < b.hi) {
        return -1;
    }
    if (a.lo > b.lo) {
        return 1;
    }
    if (a.lo < b.lo) {
        return -1;
    }
    return 0;
}
