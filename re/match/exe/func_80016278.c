#include "ac1/ac1.h"

extern s32 D_80049F94;

s32 func_80016278(s32 arg0)
{
    return *(volatile s32 *)&D_80049F94 - arg0;
}
