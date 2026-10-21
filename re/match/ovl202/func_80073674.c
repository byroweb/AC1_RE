#include "ac1/ac1.h"

extern u8 *D_801E8B68[];

void *func_80073674(s32 size)
{
    u8 buf[40];
    s32 *p;

    p = (s32 *)D_801E8B68[0];
    D_801E8B68[0] = (u8 *)p + ((size + 0xB) & ~3);
    p[0] = 0;
    p[1] = 0;
    return p;
}
