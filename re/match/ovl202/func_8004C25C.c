#include "ac1/ac1.h"
/* ovl202 0x8004C25C - start the timer (func_8001628C) and wait until the
 * status buffer reads 'R' or byte 7 reads 'C' (1); after 300 ticks raise
 * the D_8008D920 flag and give up (0). */

extern volatile u8 D_8004A28C[];
extern volatile s32 D_8008D91C;
extern volatile u8 D_8008D920;

s32 func_8004C25C(void)
{
    D_8008D91C = func_8001628C();
    while (D_8004A28C[0] != 'R') {
        if (D_8004A28C[7] == 'C') {
            break;
        }
        if (func_80016278(D_8008D91C) >= 300) {
            D_8008D920 = 1;
            return 0;
        }
    }
    return 1;
}
