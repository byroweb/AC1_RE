/* ovl202 0x8004F080  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004ECA8();                            /* extern */
M2C_UNK func_8004EFB8(void *, M2C_UNK);             /* extern */

s32 func_8004F080(void *arg0, M2C_UNK arg1) {
    u8 temp_v1;

    func_8004ECA8();
    func_8004EFB8(arg0, arg1);
    temp_v1 = M2C_FIELD(arg0, u8 *, 2) - 1;
    M2C_FIELD(arg0, u8 *, 2) = temp_v1;
    return temp_v1 & 0xFF;
}
