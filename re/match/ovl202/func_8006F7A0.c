#include "ac1/ac1.h"
/* ovl202 0x8006F7A0 - OR of the bucket words covering [lo, hi] of n cells (8 buckets) */

u32 func_8006F7A0(u32 *tab, s32 lo, s32 hi, s32 n)
{
    u32 m;
    s32 k;

    if (lo < 0) {
        lo = 0;
    } else if (lo == n) {
        lo--;
    }
    if (hi >= n) {
        hi = n - 1;
    }
    if (n == 0) {
        return *tab;
    }
    lo = (lo * 8) / n;
    hi = (hi * 8) / n;
    tab += lo;
    m = *tab++;
    k = hi - lo;
    while (k-- > 0) {
        m |= *tab++;
    }
    return m;
}
