#include "ac1/ac1.h"
/* ovl201 0x8008423C - previous bit of D_800485F0 set below bit from, 0xFF if none */
extern u32 D_800485F0;

u8 func_8008423C(s32 from)
{
    s16 i;
    u32 m;

    i = (u8)(from - 1);
    m = D_800485F0;
    for (; i >= 0; i--) {
        if (m & (1 << i)) {
            return i;
        }
    }
    return 0xFF;
}
