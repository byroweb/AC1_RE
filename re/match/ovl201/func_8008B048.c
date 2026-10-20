/* ovl201 0x8008B048  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8008B048(u8 *arg0, u8 *arg1) {
    s32 var_a3;
    u8 *var_a2;
    u8 *var_v1;
    u8 temp_v0;

    var_a3 = 0;
    if (*arg1 != 0) {
        var_v1 = arg1;
        var_a2 = arg0;
        do {
            temp_v0 = *var_v1;
            var_v1 += 1;
            var_a3 += 1;
            *var_a2 = temp_v0;
            var_a2 += 1;
        } while (*var_v1 != 0);
    }
    *(arg0 + var_a3) = *(arg1 + var_a3);
}
