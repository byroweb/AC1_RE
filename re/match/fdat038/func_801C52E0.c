#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);
s32 rand();                                         /* extern */

void func_801C52E0(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0x28, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
    M2C_FIELD(arg0, s8 *, 0x3E) = (s8) (rand() & 1);
}
