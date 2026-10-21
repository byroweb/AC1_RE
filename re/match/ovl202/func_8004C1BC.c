#include "ac1/ac1.h"
/* ovl202 0x8004C1BC - once func_80016278(D_8008D91C) reaches 300, raise the
 * D_8008D920 flag and return 0; 1 while still below. */

extern s32 D_8008D91C;
extern u8 D_8008D920;

s32 func_8004C1BC(void)
{
    if (func_80016278(D_8008D91C) >= 300) {
        D_8008D920 = 1;
        return 0;
    }
    return 1;
}
