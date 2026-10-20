/* ovl201 0x8006AF58  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8006AF58(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x48) = (s32) ((*M2C_FIELD(arg0, s8 **, 0x7C) * 0x44) + 6);
    M2C_FIELD(arg0, s32 *, 0x4C) = (s32) ((*M2C_FIELD(arg0, s8 **, 0x80) * 0x44) + 4);
}
