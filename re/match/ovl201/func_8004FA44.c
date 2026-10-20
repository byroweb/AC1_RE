/* ovl201 0x8004FA44  size 128  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80050AF4(M2C_UNK, M2C_UNK);            /* extern */
s32 func_800517C8();                                /* extern */
M2C_UNK func_80073740();                            /* extern */
extern M2C_UNK func_8004FAC4;

void func_8004FA44(void *arg0) {
    u16 temp_v0;

    temp_v0 = M2C_FIELD(arg0, u16 *, 0x7C) + 1;
    M2C_FIELD(arg0, u16 *, 0x7C) = temp_v0;
    if ((s16) temp_v0 >= 0x401) {
        M2C_FIELD(arg0, u16 *, 0x7C) = 0x400U;
    }
    if ((func_800517C8() << 0x10) == 0) {
        func_80073740();
        func_80050AF4(0x10, 2);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004FAC4;
    }
}
