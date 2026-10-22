/* fdat004 0x801C64AC  size 124  sweep.py best draft: ctx, closeness 0.581 aligned / 0.323 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);
extern u16 D_8019F52A;

void func_801C64AC(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if (D_8019F52A == 0xFFFF) {
        M2C_FIELD(arg0, u16 *, 0x160) = (u16) (M2C_FIELD(arg0, u16 *, 0x160) + arg4);
        return;
    }
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0xC8, -1);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
}
