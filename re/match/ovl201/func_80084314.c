#include "ac1/ac1.h"
/* ovl201 0x80084314 - number of bits of D_800485F0 set above bit from (up to bit 10) */
extern u32 D_800485F0;

u8 func_80084314(s32 from)
{
    s32 n;
    u32 m;
    s16 i;

    n = 0;
    m = D_800485F0;
    from++;
    m >>= from;
    for (i = (u8)from; i < 11; i++) {
        if (m & 1) {
            n++;
        }
        m >>= 1;
    }
    return n;
}
