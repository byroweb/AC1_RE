/* ovl202 0x80056A54  size 124  variant sweep (tools/match/variants.py): old lever: ptype var_s0 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800135E8(M2C_UNK *, void *, M2C_UNK);  /* extern */
extern M2C_UNK D_801AC964;

void func_80056A54(void) {
    u8 *var_s0;
    s32 var_s1;

    var_s0 = &D_801AC964;
    var_s1 = 0x1F;
    do {
        func_800135E8(var_s0, var_s0 + 0x14, 1);
        func_800135E8(var_s0, var_s0 + 0x28, 2);
        func_800135E8(var_s0, var_s0 + 0x3C, 3);
        var_s1 -= 1;
        var_s0 += 0x68;
    } while (var_s1 != -1);
}
