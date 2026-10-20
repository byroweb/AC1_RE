/* ovl204 0x80060AC4  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80060AC4(s32 arg0, s32 arg1) {
    s32 var_v1;

    var_v1 = 1;
    if (arg0 >= -0x4F) {
        var_v1 = (arg0 >= 0x4F) * 2;
    }
    if (arg1 < -0x3C) {
        var_v1 |= 4;
    } else if (arg1 >= 0x3C) {
        var_v1 |= 8;
    }
    return var_v1;
}
