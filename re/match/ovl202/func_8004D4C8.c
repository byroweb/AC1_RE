/* ovl202 0x8004D4C8  size 144  permuter tier (tools/loop/permute.py): levers ptype var_s0 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CE88();                            /* extern */
M2C_UNK func_8004CF0C(M2C_UNK *);                   /* extern */
extern M2C_UNK D_80041DE0;

void func_8004D4C8(void) {
    u8 *var_s0;
    s32 var_s1;
    u16 temp_v1;

    var_s0 = &D_80041DE0;
    var_s1 = 0x16;
    do {
        if (M2C_FIELD(var_s0, s8 *, 2) != -1) {
            temp_v1 = M2C_FIELD(var_s0, u16 *, 0);
            if (temp_v1 != 0) {
                if (!(temp_v1 & 0x8000)) {
                    func_8004CF0C(var_s0);
                } else {
                    M2C_FIELD(var_s0, u16 *, 0) = (u16) (temp_v1 & 0x7FFF);
                }
            }
        }
        var_s1 -= 1;
        var_s0 += 0xA;
    } while (var_s1 != -1);
    func_8004CE88();
}
