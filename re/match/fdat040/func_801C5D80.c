#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_801C5CE0(void *, void *, u8);

s32 func_801C5D80(void *arg0, s32 arg1, void **arg2, ...)
{
    if ((arg1 == 0x8100) && ((M2C_FIELD(*arg2, s32 *, 0xC) & 7) == 2)) {
        func_801C5CE0(arg0, (u8 *)arg0 + 0x10, M2C_FIELD(M2C_FIELD(arg0, void **, 0x38), u8 *, 0xA));
        return 1;
    }
    return 0;
}
