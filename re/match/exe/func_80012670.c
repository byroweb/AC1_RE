#include "ac1/ac1.h"

void func_80012670(char *buf, s32 v, s32 n)
{
    buf += n;
    *buf = 0;
    buf--;
    while (--n != -1) {
        *buf-- = v % 10 + '0';
        v /= 10;
    }
}
