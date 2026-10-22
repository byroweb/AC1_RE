#include "ac1/ac1.h"

void func_80085320(u8 digits, char *buf, s32 val)
{
    s16 i;
    s32 div;
    s32 q;
    u8 started = 0;
    u8 neg = 0;

    div = 1;
    if (val < 0) {
        neg = 1;
        val = -val;
    }
    for (i = 0; i < digits; i++) {
        div *= 10;
    }
    i = 0;
    if (div - 1 < val) {
        val = div - 1;
    }
    div = 1;
    for (; i < digits - 1; i++) {
        div *= 10;
    }
    for (i = 0; i < digits; i++) {
        q = val / div;
        if (q > 0) {
            *buf++ = q + '0';
            started = 1;
            val -= q * div;
        } else {
            if (started || i == digits - 1) {
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
