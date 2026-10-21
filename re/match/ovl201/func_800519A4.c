#include "ac1/ac1.h"
/* ovl201 0x800519A4 - format n (clamped to 999) as 3 right-aligned digits, space padded */

void func_800519A4(u8 *buf, s16 n)
{
    s16 i;
    s32 d;
    s32 q;
    u8 started;

    started = 0;
    if (n >= 1000) {
        n = 999;
    }
    d = 100;
    for (i = 0; i < 3; i++) {
        q = n / d;
        if (q > 0) {
            *buf++ = q + '0';
            started = 1;
            n -= q * d;
        } else {
            if (started || i == 2) {
                *buf = '0';
            } else {
                *buf = ' ';
            }
            buf++;
        }
        d /= 10;
    }
    *buf = 0;
}
