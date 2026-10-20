/* ovl201 0x800A50C4  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, M2C_UNK);            /* extern */
s32 func_800517C8();                                /* extern */
extern M2C_UNK func_800A5118;

void func_800A50C4(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_80051450(4, 1);
        func_80051450(4, 4);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A5118;
    }
}
