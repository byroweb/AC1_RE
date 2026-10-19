#include "ac1_types.h"
/* exe 0x800125D0  size 160: signed int to decimal string (max 6 digits) */
int func_800125D0(char *buf, int n)
{
    int div;
    int len;
    int started;
    int i;
    int q;

    div = 100000;
    len = 0;
    started = 0;
    if (n < 0) {
        *buf++ = '-';
        len = 1;
        n = -n;
    }
    for (i = 0; i < 6; i++) {
        q = n / div;
        n = n % div;
        if (q != 0 || started != 0 || i == 5) {
            *buf++ = q + '0';
            len++;
            started = 1;
        }
        div /= 10;
    }
    *buf = 0;
    return len;
}
