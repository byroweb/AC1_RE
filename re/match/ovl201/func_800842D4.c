#include "ac1/ac1.h"
/* ovl201 0x800842D4 - number of bits set among the 11 in D_800485F0 */
extern u32 D_800485F0;

u8 func_800842D4(void)
{
    u8 n;
    u32 m;
    s16 i;

    n = 0;
    m = D_800485F0;
    for (i = 0; i < 11; i++) {
        if (m & 1) {
            n++;
        }
        m >>= 1;
    }
    return n;
}
