#include "ac1/ac1.h"
/* ovl201 0x800841E8 - next bit of D_800485F0 set after bit from, 0xFF if none */
extern u32 D_800485F0;

u8 func_800841E8(s32 from)
{
    u32 m;
    s16 i;

    m = D_800485F0;
    from++;
    m >>= from;
    for (i = (u8)from; i < 11; i++, m >>= 1) {
        if (m & 1) {
            return i;
        }
    }
    return 0xFF;
}
