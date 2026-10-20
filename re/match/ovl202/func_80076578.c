/* ovl202 0x80076578  size 112  variant sweep (tools/match/variants.py): args lever: args func_80076548 +arg0 proto, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_80076548(void *, M2C_UNK, M2C_UNK);            /* extern */

void func_80076578(void *arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3) {
    M2C_FIELD(arg0, s16 *, 0x94) = 0;
    M2C_FIELD(arg0, s16 *, 0x8C) = 0;
    M2C_FIELD(arg0, s16 *, 0x84) = 0;
    func_80076548(arg0, 0, arg3);
    func_8004E98C(arg0 + 0x64, arg1, arg2, 0);
    M2C_FIELD(arg0, s8 *, 0x66) = 2;
}
