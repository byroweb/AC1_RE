/* fdat024 0x801C674C  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78(u8);                              /* extern */
M2C_UNK **func_80073674(M2C_UNK);                   /* extern */
extern M2C_UNK func_801C6498;

void func_801C674C(void *arg0) {
    M2C_UNK **temp_v0;

    temp_v0 = func_80073674(4);
    M2C_FIELD(arg0, M2C_UNK ***, 0x14) = temp_v0;
    *temp_v0 = &func_801C6498;
    M2C_FIELD(M2C_FIELD(arg0, M2C_UNK ***, 0x14), s32 *, 8) = func_80014D78(M2C_FIELD(arg0, u8 *, 0x28));
}
