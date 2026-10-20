/* ovl201 0x800B04A0  size 28  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800B04A0(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x48) = (s32) (*M2C_FIELD(arg0, u8 **, 0x7C) + 7);
}
