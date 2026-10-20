/* ovl201 0x800B4E84  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK ResetCallback();                            /* extern */
M2C_UNK func_800B514C(s32);                         /* extern */

void func_800B4E84(s32 arg0) {
    if (arg0 == 0) {
        ResetCallback();
    }
    func_800B514C(arg0);
}
