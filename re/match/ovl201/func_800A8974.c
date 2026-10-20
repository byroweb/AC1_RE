/* ovl201 0x800A8974  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005B648(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */

void func_800A8974(void *arg0) {
    M2C_FIELD(arg0, s8 *, 0x81) = 1;
    M2C_FIELD(arg0, s8 *, 0x80) = 0;
    M2C_FIELD(arg0, s8 *, 0x7C) = 0;
    func_8005B648(0, 0, 0x140, 0xF0, 1, 0, 0);
}
