/* ovl202 0x80083928  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80087798(s32, void *, void *, u8, s32); /* extern */

void func_80083928(void *arg0) {
    u8 temp_a0;

    temp_a0 = M2C_FIELD(arg0, u8 *, 0x21);
    if ((temp_a0 % (u8) M2C_FIELD(arg0, u8 *, 0x1E)) == 0) {
        func_80087798(M2C_FIELD(arg0, s32 *, 8), arg0, arg0 + 0x18, M2C_FIELD(arg0, u8 *, 0x1F), temp_a0 == M2C_FIELD(arg0, u8 *, 0x20));
    }
    M2C_FIELD(arg0, u8 *, 0x21) = (u8) (M2C_FIELD(arg0, u8 *, 0x21) + 1);
}
