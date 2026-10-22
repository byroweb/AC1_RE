#include "ac1/ac1.h"

extern s32 D_80049F94;

s32 func_8001628C(void)
{
    return *(volatile s32 *)&D_80049F94;
}
