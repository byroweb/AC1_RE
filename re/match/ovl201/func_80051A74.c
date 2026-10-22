#include "ac1/ac1.h"
/* format n (clamped to +-9999999) right-aligned in eight characters */

void func_80051A74(char *buf, s32 n)
{
    u8 any = 0;
    u8 neg = 0;
    s32 div;
    s32 d;
    s16 i;

    if (n < 0) {
        neg = 1;
        n = -n;
    }
    if (n > 9999999) {
        n = 9999999;
    }
    div = 10000000;
    for (i = 0; i < 8; i++) {
        d = n / div;
        if (d > 0) {
            *buf++ = d + '0';
            any = 1;
            n -= d * div;
        } else {
            if (any || i == 7) {
                *buf = '0';
            } else {
                *buf = ' ';
            }
            buf++;
        }
        div /= 10;
    }
    *buf = 0;
    if (neg) {
        buf--;
        while (*buf-- != ' ') {
        }
        buf[1] = '-';
    }
}
