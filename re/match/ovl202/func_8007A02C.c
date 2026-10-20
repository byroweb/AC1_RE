/* ovl202 0x8007A02C  size 44  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u16 D_8004125E;

void func_8007A02C(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0x148) = (u16) (M2C_FIELD(arg0, u16 *, 0x148) & 0xFFF0);
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) & ~D_8004125E);
}
