#include "ac1/ac1.h"
/* ovl203 0x8004FB8C - remap one tile / attribute pair (0x7980..0x7B80 +0x240, 0x7F90 +0x40; clear bit 4 of attributes 27 and 28) */

void func_8004FB8C(u16 *a, u16 *b)
{
    u16 x;
    u16 y;

    x = *a;
    y = *b;
    if ((u16)(x - 0x7980) <= 0x200) {
        *a = x + 0x240;
    } else if (x == 0x7F90) {
        *a = x + 0x40;
    }
    if ((u16)((y & 0x1F) - 27) < 2) {
        *b = y & 0xFFEF;
    }
}
