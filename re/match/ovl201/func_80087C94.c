#include "ac1/ac1.h"

extern char D_8004CBC4[7];

void func_80087C94(s32 n, char *buf)
{
    s32 i;

    buf[6] = 0;
    buf[5] = 'C';
    buf[0] = n % 100000 / 10000 + '0';
    buf[1] = n % 10000 / 1000 + '0';
    buf[4] = n % 10 + '0';
    buf[2] = n % 1000 / 100 + '0';
    buf[3] = n % 100 / 10 + '0';
    for (i = 0; buf[i] != 0; i++) {
        if (buf[i] != '0') break;
        buf[i] = ' ';
    }
    if (buf[i] == 'C') {
        __builtin_memcpy(buf, D_8004CBC4, 7);
    }
}
