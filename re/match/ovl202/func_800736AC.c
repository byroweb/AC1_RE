/* ovl202 0x800736AC  size 104  permuter tier (tools/loop/permute.py): levers ptype var_s0 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_80198B08;

void func_800736AC(void) {
    M2C_UNK (**temp_v0)(M2C_UNK *);
    M2C_UNK (*temp_v0_2)(M2C_UNK *);
    u8 *var_s0;
    s32 var_s1;

    var_s1 = 0x200;
    var_s0 = &D_80198B08;
    do {
        temp_v0 = M2C_FIELD(var_s0, M2C_UNK (***)(M2C_UNK *), 0x14);
        if (temp_v0 != NULL) {
            temp_v0_2 = *temp_v0;
            if (temp_v0_2 != NULL) {
                temp_v0_2(var_s0);
            }
        }
        var_s1 -= 1;
        var_s0 += 0x34;
    } while (var_s1 != 0);
}
