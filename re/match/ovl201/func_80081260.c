/* ovl201 0x80081260  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80081260(u8 *arg0) {
    s32 var_a1;
    u8 *var_a0;

    var_a0 = arg0;
    var_a1 = 0;
    if (*var_a0 != 0x3E) {
        do {
            var_a0 += 2;
            var_a1 += 1;
        } while (*var_a0 != 0x3E);
    }
    return var_a1 & 0xFF;
}
