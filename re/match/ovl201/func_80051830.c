/* ovl201 0x80051830  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK free2(s32);                                 /* extern */
M2C_UNK func_80050944();                            /* extern */

void func_80051830(s32 arg0) {
    func_80050944();
    DrawSync(0);
    free2(arg0);
}
