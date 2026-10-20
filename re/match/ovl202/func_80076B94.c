/* ovl202 0x80076B94  size 180  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004DEAC(void *, s32, M2C_UNK *, M2C_UNK *); /* extern */
s32 func_8004E22C(void *, void *, void *, M2C_UNK *, M2C_UNK *); /* extern */
M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_80076548(void *, M2C_UNK, M2C_UNK);    /* extern */

void func_80076B94(void *arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3) {
    union { s8 b[0x10]; s32 w; } sp_frame;
    void *temp_a1;
    void *temp_s0;

    if (M2C_FIELD(arg0, u8 *, 0x3D) != 0x10) {
        M2C_FIELD(arg0, s16 *, 0x1C) = 0;
        M2C_FIELD(arg0, s16 *, 0x1A) = 0;
        M2C_FIELD(arg0, s16 *, 0x18) = 0;
    }
    func_80076548(arg0, 0x10, arg3);
    temp_s0 = arg0 + 0x64;
    func_8004E98C(temp_s0, arg1, arg2, 0);
    temp_a1 = arg0 + 0x10;
    func_8004DEAC(temp_s0, func_8004E22C(temp_s0, temp_a1, temp_a1, &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8))), &(*(M2C_UNK *)(sp_frame.b + 0x0)), &(*(M2C_UNK *)(sp_frame.b + 0x8)));
}
