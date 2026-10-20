/* exe 0x8001654C  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80016190(s32);                             /* extern */
M2C_UNK func_80016328(void *, s32, s32);            /* extern */

s32 func_8001654C(void *arg0) {
    s32 temp_s0;
    s32 temp_v0;

    temp_s0 = (M2C_FIELD(arg0, s32 *, 4) + 0x7FF) & ~0x7FF;
    temp_v0 = func_80016190(temp_s0);
    func_80016328(arg0, temp_v0, temp_s0 >> 0xB);
    return temp_v0;
}
