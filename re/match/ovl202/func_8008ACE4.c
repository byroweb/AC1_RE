#include "ac1/ac1.h"
/* ovl202 0x8008ACE4: signed int to digit-code string (10 digits, '-' = 0xA) */
int func_8008ACE4(char *buf, int n)
{
    int div;
    int len;
    int started;
    int i;
    int q;

    div = 1000000000;
    len = 0;
    started = 0;
    if (n < 0) {
        *buf++ = 0xA;
        len = 1;
        n = -n;
    }
    for (i = 0; i < 10; i++) {
        q = n / div;
        n = n % div;
        if (q != 0 || started != 0 || i == 9) {
            *buf++ = q;
            len++;
            started = 1;
        }
        div /= 10;
    }
    *buf = 0;
    return len;
}
