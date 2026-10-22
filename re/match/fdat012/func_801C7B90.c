/* fdat012 0x801C7B90  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_8019FB94;

void func_801C7B90(s32 *arg0) {
    s32 var_v1;
    u8 *var_a1;

    var_a1 = &D_8019FB94;
    var_v1 = 0xFB;
    do {
        if (*var_a1 == 3) {
            *arg0 += 0x320;
        }
        var_v1 -= 1;
        var_a1 += 0x2C;
    } while (var_v1 != 0);
}
