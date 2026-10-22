#include "ac1/ac1.h"

s32 func_80014930(s32 a, s32 b)
{
    s32 d;

    d = (b - a) & 0xFFF;
    if (d > 0x800) {
        return d - 0x1000;
    }
    return d;
}
