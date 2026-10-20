/* ovl201 0x8008F344  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, s32);     /* extern */
M2C_UNK func_8008EE88(M2C_UNK *, s32);              /* extern */
s32 func_8008EED4(s32, s32);                        /* extern */
extern M2C_UNK D_8004CD20;

void func_8008F344(s32 arg0, s32 arg1, s32 arg2) {
    union { s8 b[0x10]; s32 w; } sp_frame;
    s32 temp_s0;

    func_8008EE88(&(*(M2C_UNK *)(sp_frame.b + 0x0)), func_8008EED4(arg0 & 0xFF, arg1 & 0xFF));
    temp_s0 = (arg2 & 0xFF) * 0x10;
    func_8005D15C(&D_8004CD20, 0x380, temp_s0);
    func_8005D15C(&(*(M2C_UNK *)(sp_frame.b + 0x0)), 0x380, temp_s0);
    DrawSync(0);
}
