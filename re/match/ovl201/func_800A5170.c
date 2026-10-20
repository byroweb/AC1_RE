/* ovl201 0x800A5170  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_80073904();                            /* extern */
extern M2C_UNK func_800A51B8;

void func_800A5170(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_80073904();
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A51B8;
    }
}
