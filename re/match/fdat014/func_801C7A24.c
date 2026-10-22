/* fdat014 0x801C7A24  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK **func_80073674(M2C_UNK);                   /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK func_801C7900;

void func_801C7A24(void *arg0) {
    M2C_UNK **temp_v0;

    temp_v0 = func_80073674(1);
    M2C_FIELD(arg0, M2C_UNK ***, 0x14) = temp_v0;
    *temp_v0 = &func_801C7900;
    M2C_FIELD(M2C_FIELD(arg0, M2C_UNK ***, 0x14), s8 *, 8) = (s8) (rand() & 7);
}
