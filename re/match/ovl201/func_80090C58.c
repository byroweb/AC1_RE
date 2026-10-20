/* ovl201 0x80090C58  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051628();                            /* extern */
s32 func_800517C8();                                /* extern */

void func_80090C58(void *arg0) {
    if ((func_800517C8() << 0x10) != 0) {
        func_80051628();
    }
    M2C_FIELD(arg0, s8 *, 1) = 1;
}
