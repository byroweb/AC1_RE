/* ovl201 0x8004F294  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004F294(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x58) = (s32) (M2C_FIELD(arg0, s32 *, 0x58) - 0x20);
    M2C_FIELD(arg0, s32 *, 0x5C) = (s32) (M2C_FIELD(arg0, s32 *, 0x5C) - 0x20);
    if (M2C_FIELD(arg0, s32 *, 0x58) < 0x801) {
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
