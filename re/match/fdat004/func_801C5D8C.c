#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80073714(void *, u16);

s32 func_801C5D8C(void *arg0, s32 arg1, ...)
{
    M2C_FIELD(arg0, s16 *, 0x7C) = -1;
    if (func_80073714(arg0, M2C_FIELD(arg0, u16 *, 0xFC)) != -1) {
        M2C_FIELD(arg0, s16 *, 0x7C) = 0;
    }
    return 0;
}
