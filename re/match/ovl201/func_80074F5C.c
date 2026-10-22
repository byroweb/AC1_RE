#include "ac1/ac1.h"
/* ovl201 0x80074F5C - format n (at most 999999) right-aligned in six
 * characters; 0xFFFF gives "INFINITY", -1 "------", and a blank last digit
 * becomes '0'. */

typedef struct {
    char s[9];
} Str9;

extern Str9 D_8004BF60;         /* "INFINITY" */

void func_80074F5C(s32 n, char *buf)
{
    u8 any = 0;
    s32 div;
    s32 d;
    s16 i;

    if (n == 0xFFFF) {
        *(Str9 *)buf = D_8004BF60;
    } else {
        if (n == -1) {
            for (div = 0; div < 6; div++) {
                *buf++ = '-';
            }
        } else {
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
                    if (any) {
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
    if (buf[-1] == ' ') {
        buf[-1] = '0';
    }
}
