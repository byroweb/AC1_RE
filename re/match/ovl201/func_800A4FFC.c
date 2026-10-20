/* ovl201 0x800A4FFC  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005B648(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */

void func_800A4FFC(void *arg0) {
    M2C_FIELD(arg0, s8 *, 0x8A) = 0;
    func_8005B648(0, 0, 0x140, 0xF0, 1, 0, 0);
}
