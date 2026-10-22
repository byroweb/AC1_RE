#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, s32, s32, s32, s32);
s32 func_80075750(void *, u8);                      /* extern */
M2C_UNK func_8007790C(void *, M2C_UNK, M2C_UNK, M2C_UNK, u16 *, s32, M2C_UNK *); /* extern */
extern M2C_UNK D_800764C4;

void func_801C50AC(void *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0xA, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
    if (M2C_FIELD(arg1, u8 *, 4) != 0) {
        M2C_FIELD(arg0, s8 *, 0x3C) = 3;
        M2C_FIELD(arg0, u8 *, 0x64) = 0xFF;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
        (*(u16 *)(sp_frame.b + 0x0)) <<= 6;
        (*(u16 *)(sp_frame.b + 0x2)) <<= 6;
        (*(u16 *)(sp_frame.b + 0x4)) <<= 6;
        func_8007790C(arg0, 1, 1, 0x400, &(*(u16 *)(sp_frame.b + 0x0)), func_80075750(arg0, M2C_FIELD(arg1, u8 *, 4)), &D_800764C4);
    }
}
