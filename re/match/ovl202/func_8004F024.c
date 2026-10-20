/* ovl202 0x8004F024  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EC4C(void *);                      /* extern */
M2C_UNK func_8004EDAC();                            /* extern */
M2C_UNK func_8004EFB8(void *, M2C_UNK);             /* extern */

s32 func_8004F024(void *arg0, M2C_UNK arg1) {
    u8 temp_v1;

    func_8004EDAC();
    func_8004EC4C(arg0);
    func_8004EFB8(arg0, arg1);
    temp_v1 = M2C_FIELD(arg0, u8 *, 2) - 1;
    M2C_FIELD(arg0, u8 *, 2) = temp_v1;
    return temp_v1 & 0xFF;
}
