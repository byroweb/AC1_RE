#include "ac1/ac1.h"

s32 func_80014DFC(s32 n, u8 *w, s32 stride)
{
    s32 sum;
    s32 i;
    s32 r;
    s32 acc;
    u8 *p;

    p = w;
    sum = 0;
    for (i = n - 1; i != -1; i--) {
        sum += *p;
        p += stride;
    }
    r = (rand() * sum) >> 15;
    p = w;
    acc = 0;
    for (i = 0; i < n; i++) {
        acc += *p;
        p += stride;
        if (r < acc) {
            return i;
        }
    }
    return -1;
}
