#include "ac1/ac1.h"

s32 func_80015B24(s32 *buf, s32 n)
{
    s32 *p;
    s32 sum;
    s32 i;
    s32 last;

    sum = 0x12345678;
    p = buf;
    n <<= 9;
    last = n - 1;
    i = n - 2;
    if (i != -1) {
        do {
            sum += *p++;
            i--;
        } while (i != -1);
    }
    return sum != buf[last];
}
