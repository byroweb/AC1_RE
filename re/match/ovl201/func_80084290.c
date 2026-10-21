#include "ac1/ac1.h"
/* ovl201 0x80084290 - index of the lowest of the 11 bits set in D_800485F0, 0xFF if none */
extern u32 D_800485F0;

u8 func_80084290(void)
{
    s16 i;
    u32 m;

    m = D_800485F0;
    for (i = 0; i < 11; i++, m >>= 1) {
        if (m & 1) {
            return i;
        }
    }
    return 0xFF;
}
