#include "ac1/ac1.h"
/* ovl202 0x8006E780 - clear a 16-word block, then func_8006E718 on it */

void func_8006E718(s32 *p);

void func_8006E780(s32 *arg0)
{
    s32 *p;
    s32 i;

    p = arg0;
    i = 16;
    do {
        *p = 0;
        i -= 1;
        p += 1;
    } while (i != 0);
    func_8006E718(arg0);
}
