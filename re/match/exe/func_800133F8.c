#include "ac1/ac1.h"

s32 func_800133F8(s32 a, s32 b)
{
    if (a > 0) {
        a += b;
        if (a < 0) {
            a = 0;
        }
    } else {
        a += b;
        if (a > 0) {
            a = 0;
        }
    }
    return a;
}
