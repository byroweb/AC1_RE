/* fdat074 0x801C7220  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_801C71D4(M2C_UNK);                         /* extern */

void func_801C7220(s32 arg0, s32 *arg1) {
    s32 temp_s0;

    temp_s0 = func_801C71D4(2);
    if ((temp_s0 + func_801C71D4(3)) == 0) {
        *arg1 -= 0x1388;
    }
}
