/* fdat016 0x801C85FC  size 64  permuter tier (tools/loop/permute.py): levers bytes var_a1 += 0x2C, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_8019FB94;

void func_801C85FC(s32 *arg0) {
    M2C_UNK *var_a1;
    s32 var_v1;

    var_a1 = &D_8019FB94;
    var_v1 = 5;
loop_1:
    if (M2C_FIELD(var_a1, u16 *, 2) != 0) {
        var_v1 -= 1;
        var_a1 = (M2C_UNK *)((u8 *)var_a1 + 0x2C);
        if (var_v1 == 0) {
            *arg0 += 0x2710;
        } else {
            goto loop_1;
        }
    }
}
