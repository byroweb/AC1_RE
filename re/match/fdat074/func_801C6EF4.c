/* fdat074 0x801C6EF4  size 172  permuter tier (tools/loop/permute.py): levers bytes var_s0 += 0x170, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rand();                                         /* extern */
extern M2C_UNK D_801A26B8;

M2C_UNK *func_801C6EF4(void) {
    M2C_UNK *var_s0;
    M2C_UNK *var_s3;
    s32 temp_v0;
    s32 var_s1;
    s32 var_s2;

    var_s0 = &D_801A26B8;
    var_s3 = NULL;
    var_s1 = 0x10;
    var_s2 = -1;
    do {
        if ((M2C_FIELD(var_s0, s32 *, 0) != 0) && ((M2C_FIELD(var_s0, u16 *, 0x36) & 7) == 2)) {
            temp_v0 = rand();
            if (var_s2 < temp_v0) {
                var_s2 = temp_v0;
                var_s3 = var_s0;
            }
        }
        var_s1 -= 1;
        var_s0 = (M2C_UNK *)((u8 *)var_s0 + 0x170);
    } while (var_s1 != 0);
    return var_s3;
}
