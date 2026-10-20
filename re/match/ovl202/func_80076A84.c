/* ovl202 0x80076A84  size 272  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004DEAC(void *, s32, M2C_UNK *, M2C_UNK *); /* extern */
s32 func_8004E22C(void *, void *, void *, M2C_UNK *, M2C_UNK *); /* extern */
M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8004EA3C(void *, M2C_UNK, s32);        /* extern */
s32 func_800749A0(void *, void *, M2C_UNK *, M2C_UNK *); /* extern */
M2C_UNK func_80076548(void *, M2C_UNK, s32);        /* extern */

void func_80076A84(void *arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3, s32 arg4, s32 arg5) {
    union { s8 b[0x18]; s32 w; } sp_frame;
    M2C_UNK (*temp_v0)(void *, M2C_UNK, s32);
    s32 temp_s0;
    s32 temp_s0_2;
    void *temp_a1;
    void *temp_s1;

    if (M2C_FIELD(arg0, u8 *, 0x3D) != 0x10) {
        M2C_FIELD(arg0, s16 *, 0x1C) = 0;
        M2C_FIELD(arg0, s16 *, 0x1A) = 0;
        M2C_FIELD(arg0, s16 *, 0x18) = 0;
    }
    func_80076548(arg0, 0x10, arg5);
    temp_s1 = arg0 + 0x64;
    func_8004E98C(temp_s1, arg1, arg2, 0);
    func_8004EA3C(temp_s1, arg3, arg4);
    temp_a1 = arg0 + 0x10;
    temp_s0 = func_8004E22C(temp_s1, temp_a1, temp_a1, &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
    temp_s0_2 = func_800749A0(arg0, arg0 + 8, &(*(M2C_UNK *)(sp_frame.b + 0x10)), &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    func_8004DEAC(temp_s1, temp_s0, &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
    if (temp_s0_2 != 0) {
        temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK, s32), 0x54);
        if (temp_v0 != NULL) {
            temp_v0(arg0, 5, temp_s0_2);
        }
    }
}
