/* ovl202 0x8008D668  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8008D668(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 var_a0;

    var_a0 = arg0 + arg1;
    if (arg1 >= 0) {
        if (arg3 < var_a0) {
            var_a0 = arg3;
        }
    } else if (var_a0 < arg2) {
        var_a0 = arg2;
    }
    return var_a0;
}
