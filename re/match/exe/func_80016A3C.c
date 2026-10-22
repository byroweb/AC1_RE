#include "ac1/ac1.h"

s32 func_80016A3C(u8 *p)
{
    if (p[0] != 0) {
        return 0;
    }
    if ((p[1] & 0xF0) != 0x40) {
        return 0;
    }
    return (u16)~((p[2] << 8) | p[3]);
}
