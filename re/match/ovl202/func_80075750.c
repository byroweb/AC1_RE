/* ovl202 0x80075750  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80075750(void *arg0, s32 arg1) {
    s32 var_v1;

    var_v1 = (s32) (arg1 * 0x32) / (s32) M2C_FIELD(arg0, u16 *, 0x164);
    if (var_v1 <= 0) {
        var_v1 = 1;
    } else if (var_v1 >= 0x1F) {
        var_v1 = 0x1E;
    }
    return var_v1;
}
