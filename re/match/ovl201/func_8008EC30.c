#include "ac1/ac1.h"
/* ovl201 0x8008EC30 - format n as six digits plus 'C' (credits), leading
 * zeros blanked, at least one digit kept. */

void func_8008EC30(s32 n, char *buf)
{
    s32 i;

    i = 0;
    buf[0] = n % 1000000 / 100000 + '0';
    buf[1] = n % 100000 / 10000 + '0';
    buf[2] = n % 10000 / 1000 + '0';
    buf[3] = n % 1000 / 100 + '0';
    buf[4] = n % 100 / 10 + '0';
    buf[5] = n % 10 + '0';
    buf[6] = 'C';
    buf[7] = 0;
    for (; buf[i] != 0; i++) {
        if (buf[i] != '0') {
            break;
        }
        buf[i] = ' ';
    }
    if (buf[i] == 0) {
        buf[i - 1] = '0';
    }
}
