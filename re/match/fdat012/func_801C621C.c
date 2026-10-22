#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_80075418(void *, void *, s32, s32, s32);
extern s32 func_80075750(void *, s32);
extern s32 func_80075798(void *, void *, s32, s32, s32, SVECTOR *);
extern void func_800775B8(void *, s32, s32, s32, SVECTOR *, void *);
extern void func_801C5600();

void func_801C621C(void *arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 anim;
    s32 q;

    M2C_FIELD(arg0, u8 *, 0xFE) = 10;
    if (arg1[4] != 0) {
        M2C_FIELD(arg0, u8 *, 0x3C) = 0x81;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
        q = (func_80075798(arg0, arg1, arg2, arg3, 0xF33, (SVECTOR *)((u8 *)arg0 + 0xCC)) - 0x200) & 0xFFF;
        M2C_FIELD(arg0, u16 *, 0xCC) <<= 6;
        M2C_FIELD(arg0, u16 *, 0xCE) <<= 6;
        M2C_FIELD(arg0, u16 *, 0xD0) <<= 6;
        switch (q >> 10) {
        case 0:
            anim = 3;
            break;
        case 1:
            anim = 6;
            break;
        case 2:
            anim = 4;
            break;
        case 3:
            anim = 5;
            break;
        }
        M2C_FIELD(arg0, u8 *, 0x3C) = 0x81;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
        func_800775B8(arg0, anim, 0, 0x200, (SVECTOR *)((u8 *)arg0 + 0xCC), func_801C5600);
    }
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 100, 256);
        M2C_FIELD(arg0, s16 *, 0xD2) = -1;
    } else {
        M2C_FIELD(arg0, s16 *, 0xD2) = func_80075750(arg0, arg4);
    }
}
