#include "ac1/ac1.h"
/* ovl202 0x8006EFFC - do the ranges [a0,a1] and [b0,b1] overlap */

s32 func_8006EFFC(s32 a0, s32 a1, s32 b0, s32 b1)
{
    if (a0 < b0) {
        return a1 >= b0;
    }
    if (b0 < a0) {
        return b1 >= a0;
    }
    return 1;
}
