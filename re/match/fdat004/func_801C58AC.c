#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_80075418(void *, void *, s32, s32, s32);
extern s32 func_80075750(void *, s32);
extern s32 func_80075798(void *, void *, s32, s32, s32, SVECTOR *);
extern void func_800775B8(void *, s32, s32, s32, SVECTOR *, void *);
extern void func_801C4D30();

void func_801C58AC(void *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    SVECTOR *v;

    if (M2C_FIELD(arg0, u8 *, 0x3D) == 0xC0) {
        v = (SVECTOR *)((u8 *)arg0 + 0xCC);
        M2C_FIELD(arg0, u8 *, 0x3C) = 0x81;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
        func_80075798(arg0, arg1, arg2, arg3, 0xF33, v);
        v->vx <<= 6;
        v->vy <<= 6;
        v->vz <<= 6;
        func_800775B8(arg0, 0, 0, 0x200, v, func_801C4D30);
        if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
            func_80075418(arg0, arg1, arg4, 100, -1);
            M2C_FIELD(arg0, s16 *, 0xD2) = -1;
            return;
        }
        M2C_FIELD(arg0, s16 *, 0xD2) = func_80075750(arg0, arg4);
        return;
    }
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        func_80075418(arg0, arg1, arg4, 50, 0x100);
        M2C_FIELD(arg0, s8 *, 0x3C) = 0;
        M2C_FIELD(arg0, u16 *, 0x36) |= 0x10;
    }
}
