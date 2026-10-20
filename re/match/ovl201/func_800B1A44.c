/* ovl201 0x800B1A44  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800B1A44(void *arg0) {
    M2C_FIELD(arg0, s32 *, 0x48) = (s32) (*M2C_FIELD(arg0, u8 **, 0x7C) * 0x90);
    M2C_FIELD(arg0, s32 *, 0x4C) = (s32) ((*M2C_FIELD(arg0, u8 **, 0x80) * 0x10) + 0xC);
}
