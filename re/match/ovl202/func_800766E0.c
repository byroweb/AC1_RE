/* ovl202 0x800766E0  size 248  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004DEAC(void *, s32, M2C_UNK *, M2C_UNK *); /* extern */
s32 func_8004E22C(void *, void *, void *, M2C_UNK *, M2C_UNK *); /* extern */
M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8004EA3C(void *, s32, s32);            /* extern */
M2C_UNK func_8004EA98(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_80075108(void *, M2C_UNK *);           /* extern */
M2C_UNK func_80076548(void *, M2C_UNK, s32);        /* extern */

void func_800766E0(void *arg0, M2C_UNK arg1, M2C_UNK arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    union { s8 b[0x10]; s32 w; } sp_frame;
    s32 temp_s0_2;
    void *temp_a1;
    void *temp_s0;
    void *temp_s1;

    if (M2C_FIELD(arg0, u8 *, 0x3D) != 0x10) {
        M2C_FIELD(arg0, s16 *, 0x1C) = 0;
        M2C_FIELD(arg0, s16 *, 0x1A) = 0;
        M2C_FIELD(arg0, s16 *, 0x18) = 0;
    }
    func_80076548(arg0, 0x10, arg6);
    if (arg3 != 0) {
        func_8004EA98(arg0 + 0x64, arg1, arg2, 0);
    } else {
        temp_s0 = arg0 + 0x64;
        func_8004E98C(temp_s0, arg1, arg2, 0);
        func_8004EA3C(temp_s0, arg4, arg5);
    }
    temp_s1 = arg0 + 0x64;
    temp_a1 = arg0 + 0x10;
    temp_s0_2 = func_8004E22C(temp_s1, temp_a1, temp_a1, &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
    func_80075108(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    func_8004DEAC(temp_s1, temp_s0_2, &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
}
