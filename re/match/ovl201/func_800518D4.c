#include "ac1/ac1.h"
/* ovl201 0x800518D4 - format n (clamped to 99) as 2 right-aligned digits, space padded */

void func_800518D4(u8 *buf, s16 n)
{
    s16 i;
    s32 d;
    s32 q;
    u8 started;

    started = 0;
    if (n >= 100) {
        n = 99;
    }
    d = 10;
    for (i = 0; i < 2; i++) {
        q = n / d;
        if (q > 0) {
            *buf++ = q + '0';
            started = 1;
            n -= q * d;
        } else {
            if (started || i == 1) {
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
