/* ovl202 0x800558A8  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80016114(s32);                         /* extern */

void func_800558A8(void *arg0) {
    s32 temp_a0;

    M2C_FIELD(arg0, s16 *, 0) = 0;
    *M2C_FIELD(arg0, s32 **, 0x24) = 0;
    temp_a0 = M2C_FIELD(arg0, s32 *, 0x20);
    if (temp_a0 != 0) {
        func_80016114(temp_a0);
        M2C_FIELD(arg0, s32 *, 0x20) = 0;
    }
}
