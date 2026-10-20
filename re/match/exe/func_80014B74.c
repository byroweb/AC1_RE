/* exe 0x80014B74  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014950(s32, s32);                        /* extern */
s16 ratan2(s32, s32);                               /* extern */

void func_80014B74(void *arg0, void *arg1, void *arg2) {
    s32 temp_s0;
    s32 temp_s1;
    s32 temp_s2;

    temp_s1 = M2C_FIELD(arg1, s16 *, 0) - M2C_FIELD(arg0, s16 *, 0);
    temp_s0 = M2C_FIELD(arg1, s16 *, 4) - M2C_FIELD(arg0, s16 *, 4);
    temp_s2 = M2C_FIELD(arg1, s16 *, 2) - M2C_FIELD(arg0, s16 *, 2);
    M2C_FIELD(arg2, s16 *, 2) = (s16) -ratan2(temp_s1, temp_s0);
    M2C_FIELD(arg2, s16 *, 0) = (s16) (ratan2(temp_s2, func_80014950(temp_s1, temp_s0)) & 0xFFF);
    M2C_FIELD(arg2, s16 *, 4) = 0;
}
