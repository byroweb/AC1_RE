/* fdat014 0x801C8380  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_801C830C(M2C_UNK);                         /* extern */
extern s32 D_801C8544;

void func_801C8380(void) {
    s32 temp_s0;

    temp_s0 = func_801C830C(0x15);
    D_801C8544 = temp_s0 + func_801C830C(0x16);
}
