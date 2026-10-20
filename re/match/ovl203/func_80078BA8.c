/* ovl203 0x80078BA8  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void *D_80039D18;

void func_80078BA8(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x15C) = (s32) (M2C_FIELD(arg0, s32 *, 0x15C) ^ (1 << M2C_FIELD(D_80039D18, u8 *, 0x2AB)));
}
