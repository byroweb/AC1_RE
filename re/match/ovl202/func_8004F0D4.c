/* ovl202 0x8004F0D4  size 204  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EC4C();                            /* extern */
s16 func_8004EF74(void *, s16, u8);                 /* extern */

s32 func_8004F0D4(void *arg0, void *arg1, void *arg2) {
    u8 temp_v1;

    func_8004EC4C();
    M2C_FIELD(arg1, s16 *, 0) = func_8004EF74(arg0 + 0x20, M2C_FIELD(arg1, s16 *, 0), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg1, s16 *, 2) = func_8004EF74(arg0 + 0x28, M2C_FIELD(arg1, s16 *, 2), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg1, s16 *, 4) = func_8004EF74(arg0 + 0x30, M2C_FIELD(arg1, s16 *, 4), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg2, s16 *, 0) = func_8004EF74(arg0 + 0x38, M2C_FIELD(arg2, s16 *, 0), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg2, s16 *, 2) = func_8004EF74(arg0 + 0x40, M2C_FIELD(arg2, s16 *, 2), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg2, s16 *, 4) = func_8004EF74(arg0 + 0x48, M2C_FIELD(arg2, s16 *, 4), M2C_FIELD(arg0, u8 *, 3));
    temp_v1 = M2C_FIELD(arg0, u8 *, 2) - 1;
    M2C_FIELD(arg0, u8 *, 2) = temp_v1;
    return temp_v1 & 0xFF;
}
