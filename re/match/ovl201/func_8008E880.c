#include "ac1/ac1.h"
/* ovl201 0x8008E880 - format signed n as eight digits, leading zeros
 * blanked (at least one digit kept), '-' just before the first digit. */

void func_8008E880(s32 n, char *buf)
{
    s32 neg;
    s32 i;
    char *p = buf;

    if (n < 0) {
        neg = 1;
        n = -n;
    } else {
        neg = 0;
    }
    i = 0;
    buf[0] = n % 100000000 / 10000000 + '0';
    buf[1] = n % 10000000 / 1000000 + '0';
    buf[2] = n % 1000000 / 100000 + '0';
    buf[3] = n % 100000 / 10000 + '0';
    buf[4] = n % 10000 / 1000 + '0';
    buf[5] = n % 1000 / 100 + '0';
    buf[6] = n % 100 / 10 + '0';
    buf[7] = n % 10 + '0';
    buf[8] = 0;
    for (; buf[i] != 0; i++) {
        if (buf[i] != '0') {
            break;
        }
        buf[i] = ' ';
    }
    if (buf[i] == 0) {
        buf[i - 1] = '0';
    }
    if (neg) {
        buf = p;
        for (i = 0; buf[i] != 0; i++) {
            if (buf[i] != ' ') {
                break;
            }
        }
        buf[i - 1] = '-';
    }
}
