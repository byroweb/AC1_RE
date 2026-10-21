#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);
s32 func_80075750(void *, u8);                      /* extern */
M2C_UNK func_80075798(void *, void *, M2C_UNK, M2C_UNK, s32, M2C_UNK *); /* extern */
M2C_UNK func_8007790C(void *, M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK *, s32, M2C_UNK *); /* extern */
extern M2C_UNK D_800764C4;

void func_801C5678(void *arg0, void *arg1, M2C_UNK arg2, M2C_UNK arg3, s32 arg4) {
    M2C_UNK sp20;

    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0x28, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
    if (M2C_FIELD(arg1, u8 *, 4) != 0) {
        func_80075798(arg0, arg1, arg2, arg3, 0xCCC, &sp20);
        M2C_FIELD(arg0, s8 *, 0x3C) = 3;
        M2C_FIELD(arg0, u8 *, 0x64) = 0xFF;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
        func_8007790C(arg0, 7, 1, 0x400, &sp20, func_80075750(arg0, M2C_FIELD(arg1, u8 *, 4)), &D_800764C4);
    }
}
