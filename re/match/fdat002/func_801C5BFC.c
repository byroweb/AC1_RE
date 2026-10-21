#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_80075418(void *, void *, s32, s32, s32);
extern s32 func_80075750(void *, u8);
extern s32 func_80075798(void *, void *, s32, s32, s32, SVECTOR *);
extern void func_8007790C(void *, s32, s32, s32, SVECTOR *, s32, void *);
extern u8 D_800764C4[];

void func_801C5BFC(void *arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    SVECTOR v;
    s32 anim;

    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 0x28, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
    }
    if (arg1[4] != 0) {
        switch (((func_80075798(arg0, arg1, arg2, arg3, 0xCCC, &v) - 0x200) & 0xFFF) >> 10) {
        case 0:
            anim = 0x16;
            break;
        case 1:
            anim = 0x18;
            break;
        case 2:
            anim = 0x17;
            break;
        case 3:
            anim = 0x19;
            break;
        }
        M2C_FIELD(arg0, s8 *, 0x3C) = 0xB;
        M2C_FIELD(arg0, u8 *, 0x64) = 0xFF;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
        v.vx <<= 6;
        v.vy <<= 6;
        v.vz <<= 6;
        func_8007790C(arg0, anim, 1, 0x400, &v, func_80075750(arg0, arg1[4]), D_800764C4);
    }
}
