/* ovl201 0x800B3E7C  size 28  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800B3E7C(void *arg0) {
    if (M2C_FIELD(arg0, s32 *, 0x6C) == 0) {
        M2C_FIELD(arg0, s16 *, 0x42) = 0x100;
    }
}
