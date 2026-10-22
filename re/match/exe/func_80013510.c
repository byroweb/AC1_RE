#include "ac1/ac1.h"

extern s32 func_800148E4(s16 a, s16 b, s16 range);
extern s32 func_80014920(s16 a, s16 b);

s32 func_80013510(s32 a, s32 b, s32 d, s32 tol, s32 step)
{
    if (func_800148E4(a, b, tol >> 1) != 0 && d <= tol && d >= -tol) {
        return a - b;
    }
    if (b != a) {
        if (func_80014920(b, a) != 0) {
            d -= tol;
        } else {
            d += tol;
        }
    }
    if (d > 0) {
        d -= step;
    } else if (d < 0) {
        d += step;
    }
    return d;
}
