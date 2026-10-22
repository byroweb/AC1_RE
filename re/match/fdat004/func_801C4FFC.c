#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void func_80076578(void *, s32, s32, void *);
extern void func_800775B8(void *, s32, s32, s32, void *, void *);
extern u8 D_80076014[];
extern s32 D_801A5DC8;
extern void func_801C4D30();

void func_801C4FFC(void *arg0, s32 arg1)
{
    s32 s;

    s = arg1;
    if (M2C_FIELD(arg0, u16 *, 0x36) & 0x10) {
        s = M2C_FIELD(arg0, u8 *, 0x3C);
    }
    D_801A5DC8 = 0;
    switch (s) {
    case 1:
        func_800775B8(arg0, 0, 1, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 2:
        func_800775B8(arg0, 1, 1, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 3:
        func_800775B8(arg0, 2, 1, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 4:
        func_800775B8(arg0, 3, 1, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 5:
        func_800775B8(arg0, 4, 1, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 0x81:
        func_800775B8(arg0, 0, 0, 0x200, (u8 *)arg0 + 0xCC, func_801C4D30);
        break;
    case 0:
    default:
        func_80076578(arg0, 0, 1, D_80076014);
        M2C_FIELD(arg0, s16 *, 0x72) = 0x200;
        break;
    }
    if (!(M2C_FIELD(arg0, u16 *, 0x36) & 0x10)) {
        M2C_FIELD(arg0, u8 *, 0x3C) = s;
    }
}
