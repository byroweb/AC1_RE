#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);

void func_801C5EF8(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0x28, 0x100);
    }
}
