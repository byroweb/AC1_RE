#include "ac1/ac1.h"

extern s32 D_801A5F2C[];
extern s32 D_801A5F30;

void func_800579D4(u16 i)
{
    s32 unused[2];      /* retail frame slot, never touched */

    D_801A5F30 = D_801A5F2C[i];
}
