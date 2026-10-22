#include "ac1/ac1.h"
/* ovl201 0x80090D60 - format n right-aligned in six characters with leading
 * blanks; 0xFFFF copies the 9-byte text at D_8004CD84, -1 gives "------".
 * A trailing blank becomes '0'. */

typedef struct {
    char c[9];
} Text9;
extern Text9 D_8004CD84;

void func_80090D60(s32 n, char *buf)
{
    u8 any = 0;
    s32 div;
    s32 d;
    s16 i;

    if (n == 0xFFFF) {
        *(Text9 *)buf = D_8004CD84;
    } else {
        if (n == -1) {
            for (div = 0; div < 6; div++) {
                *buf++ = '-';
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
