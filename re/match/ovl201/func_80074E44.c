#include "ac1/ac1.h"
/* ovl201 0x80074E44 - format n (clamped to +-999999) right-aligned in six
 * characters with leading blanks and a '-' just before the digits. */

void func_80074E44(char *buf, s32 n)
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
    if (n > 999999) {
        n = 999999;
    }
    div = 100000;
    for (i = 0; i < 6; i++) {
        d = n / div;
        if (d > 0) {
            *buf++ = d + '0';
            any = 1;
            n -= d * div;
        } else {
            if (any || i == 5) {
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
