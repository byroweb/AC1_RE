/* exe 0x80016190  size 40  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK malloc2(s32);                               /* extern */

void func_80016190(s32 arg0) {
    malloc2((arg0 + 3) & ~3);
}
