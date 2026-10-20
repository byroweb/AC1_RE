/* ovl202 0x80079DB0  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004ECA8(s32);                         /* extern */
M2C_UNK func_80079CDC(s32);                         /* extern */

void func_80079DB0(s32 arg0) {
    func_8004ECA8(arg0 + 0x64);
    func_80079CDC(arg0);
}
