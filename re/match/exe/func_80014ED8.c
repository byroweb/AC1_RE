#include "ac1/ac1.h"

s32 func_80014ED8(s32 a, s32 b, s32 t)
{
    s32 d;

    d = ((s16)b - (s16)a) & 0xFFF;
    if (d > 0x800) {
        d -= 0x1000;
    }
    return (a + ((d * t) >> 12)) & 0xFFF;
}
