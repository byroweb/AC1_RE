#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80075418(void *, void *, s32, s32, s32);
s16 func_80075750(void *, s32);                     /* extern */
M2C_UNK func_80075798(void *, void *, M2C_UNK, M2C_UNK, s32, void *); /* extern */

void func_801C5688(void *arg0, void *arg1, M2C_UNK arg2, M2C_UNK arg3, s32 arg4)
{
    u8 unused[8];

    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0x28, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) = (u16) (M2C_FIELD(arg0, u16 *, 0x36) | 0x10);
    }
    if (M2C_FIELD(arg1, u8 *, 4) != 0) {
        func_80075798(arg0, arg1, arg2, arg3, 0xCCC, (SVECTOR *)((u8 *)arg0 + 0xCC));
        ((SVECTOR *)((u8 *)arg0 + 0xCC))->vx <<= 6;
        ((SVECTOR *)((u8 *)arg0 + 0xCC))->vy <<= 6;
        ((SVECTOR *)((u8 *)arg0 + 0xCC))->vz <<= 6;
        M2C_FIELD(arg0, s16 *, 0xD2) = func_80075750(arg0, arg4);
        M2C_FIELD(arg0, s8 *, 0x64) = 0;
    }
}
