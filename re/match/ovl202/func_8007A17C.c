/* ovl202 0x8007A17C  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8007A17C(void *arg0, void *arg1) {
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) & ~(1 << M2C_FIELD(arg1, u8 *, 6)));
}
