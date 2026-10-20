/* exe 0x80013390  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80013390(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_a0;
    s32 var_v0;

    var_a0 = arg0;
    if (arg2 > 0) {
        if (var_a0 < arg1) {
            var_a0 += arg2;
            goto block_9;
        }
        if (arg1 < var_a0) {
            var_a0 -= arg2;
            var_v0 = var_a0 < arg1;
            goto block_10;
        }
    } else {
        if (arg1 < var_a0) {
            var_a0 += arg2;
            var_v0 = var_a0 < arg1;
            goto block_10;
        }
        if (var_a0 < arg1) {
            var_a0 -= arg2;
block_9:
            var_v0 = arg1 < var_a0;
block_10:
            if (var_v0 != 0) {
                var_a0 = arg1;
            }
        }
    }
    return var_a0;
}
