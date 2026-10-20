/* ovl201 0x800728A8  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800728F8();                            /* extern */

void func_800728A8(void *arg0) {
    if (M2C_FIELD(arg0, u8 *, 1) == 0) {
        func_800728F8();
    }
}
