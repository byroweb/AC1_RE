#include "ac1/ac1.h"
/* ovl201 0x80074D40 - format n right-aligned in six characters with leading
 * blanks; n == -1 gives "------". */

void func_80074D40(char *buf, s16 n)
{
    u8 any = 0;
    s32 div;
    s32 d;
    s16 i;

    if (n == -1) {
        for (i = 0; i < 6; i++) {
            *buf++ = '-';
            *buf = 0;
        }
    } else {
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
    }
    *buf = 0;
}
