/* ovl201 0x80080080  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80080080(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x4C) = (s32) (M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s8 *, 0x80) * 0x18);
}
