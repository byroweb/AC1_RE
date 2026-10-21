#include "ac1/ac1.h"

void func_80054660(s16 *p, s32 x, s32 *idx, s32 *frac) {
    s32 sum;
    s32 i;
    s32 n;
    s32 v;

    p++;
    n = *p++;
    sum = 0;
    for (i = 0; i < n; i++) {
        v = *p++;
        sum += v;
        if (x < sum) {
            *idx = i - 1;
            *frac = (((x - (sum - v)) << 12) + 0x800) / v;
            return;
        }
    }
    *idx = 0;
    *frac = 0;
}
