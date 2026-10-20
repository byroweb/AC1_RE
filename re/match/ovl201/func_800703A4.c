/* ovl201 0x800703A4  size 44  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800703A4(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x4C) = (s32) (M2C_FIELD(arg0, s16 *, 0x80) + (*M2C_FIELD(arg0, u8 **, 0x7C) * 0x18));
}
