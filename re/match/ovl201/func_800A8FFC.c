/* ovl201 0x800A8FFC  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80050AF4(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK func_80051450(M2C_UNK, s32);                /* extern */
s32 func_800517C8();                                /* extern */
extern u8 D_80039CAE;
extern M2C_UNK func_800A9070;

void func_800A8FFC(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_80050AF4(0xF, 1);
        func_80050AF4(0x10, 2);
        func_80051450(4, (D_80039CAE - 0x17) & 0xFFFF);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A9070;
    }
}
