#include "ac1/ac1.h"
#include "ac1/i64.h"

/* v << n (dir 0) or v >> n logical, one bit per step. */
U64 func_800B6214(s32 dir, U64 v, s32 n) {
    s32 i;

    if (dir == 0) {
        for (i = 0; i < n; i++) {
            v.hi <<= 1;
            if (v.lo & 0x80000000) {
                v.hi |= 1;
            }
            v.lo <<= 1;
        }
    } else {
        for (i = 0; i < n; i++) {
            v.lo >>= 1;
            if (v.hi & 1) {
                v.lo |= 0x80000000;
            }
            v.hi >>= 1;
        }
    }
    return v;
}
