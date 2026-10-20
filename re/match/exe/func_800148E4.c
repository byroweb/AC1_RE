/* exe 0x800148E4  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800148E4(s32 arg0, s32 arg1, s16 arg2) {
    s32 temp_a0;
    s32 var_v1;

    temp_a0 = (arg0 - arg1) & 0xFFF;
    var_v1 = 0;
    if ((arg2 >= temp_a0) || (temp_a0 >= (0x1000 - arg2))) {
        var_v1 = 1;
    }
    return var_v1;
}
