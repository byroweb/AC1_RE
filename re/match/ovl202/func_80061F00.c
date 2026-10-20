/* ovl202 0x80061F00  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80061F00(s32 arg0, s32 arg1) {
    s32 var_v1;

    var_v1 = 1;
    if (arg0 >= -0xA0) {
        var_v1 = (arg0 >= 0xA0) * 2;
    }
    if (arg1 < -0x78) {
        var_v1 |= 4;
    } else if (arg1 >= 0x78) {
        var_v1 |= 8;
    }
    return var_v1;
}
