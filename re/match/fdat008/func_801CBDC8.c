/* fdat008 0x801CBDC8  size 152  sweep.py best draft: ctx, closeness 0.947 aligned / 0.947 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CC30(u8, s32, s32, s32, s32, s32, s32); /* extern */
M2C_UNK func_801CA6BC();                            /* extern */
extern void *D_80040FEC;
extern s32 D_801A5DC0;

void func_801CBDC8(void *arg0) {
    s16 temp_v0;

    func_801CA6BC();
    func_8004CC30(M2C_FIELD(D_80040FEC, u8 *, 1), D_801A5DC0 + 0x1300, arg0 + 8, ((s32) (M2C_FIELD(arg0, u16 *, 0x122) << 0x10) >> 0x17) + 0x32, 0x1F40, 0x2710, (s32) (M2C_FIELD(arg0, u16 *, 0x122) << 0x10) >> 0x17);
    temp_v0 = M2C_FIELD(arg0, u16 *, 0x122) - 0x100;
    M2C_FIELD(arg0, u16 *, 0x122) = temp_v0;
    if (temp_v0 < 0) {
        M2C_FIELD(arg0, u16 *, 0x122) = 0U;
    }
}
