/* ovl201 0x800736F0  size 40  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800736F0(void *arg0) {
    if (M2C_FIELD(arg0, s32 *, 0x6C) == 0) {
        M2C_FIELD(arg0, u16 *, 0x42) = (u16) (M2C_FIELD(arg0, u16 *, 0x42) + 0x20);
    }
}
