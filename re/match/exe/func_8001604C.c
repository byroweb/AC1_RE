#include "ac1/ac1.h"

void func_8001604C(u8 *p, s32 n)
{
    p[0] = 0;
    *(s32 *)(p + 4) = n - 20;
    p += n - 4;
    *p = 0xFF;
}
