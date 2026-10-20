/* ovl201 0x800810C0  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800810C0(void *arg0) {
    void *temp_v0;

    temp_v0 = M2C_FIELD(arg0, void **, 0x7C);
    M2C_FIELD(arg0, s32 *, 0x4C) = (s32) (((M2C_FIELD(temp_v0, u8 *, 0x8C) - M2C_FIELD(temp_v0, u8 *, 0x8D)) & 0xFF) * 0x18);
}
