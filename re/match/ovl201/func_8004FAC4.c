/* ovl201 0x8004FAC4  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */

void func_8004FAC4(void *arg0) {
    u16 temp_v0;

    temp_v0 = M2C_FIELD(arg0, u16 *, 0x7C) + 1;
    M2C_FIELD(arg0, u16 *, 0x7C) = temp_v0;
    if ((s16) temp_v0 >= 0x401) {
        M2C_FIELD(arg0, u16 *, 0x7C) = 0x400U;
    }
    if ((func_800517C8() << 0x10) == 0) {
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
