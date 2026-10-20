/* ovl202 0x8006DA20  size 88  variant sweep (tools/match/variants.py): old lever: etype D_801EA790 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80015538(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
extern u8 D_801EA790;

void func_8006DA20(void) {
    func_80015538(&D_801EA790, 0, 0x400);
    func_80015538(&D_801EA790 + 0x1000, 0, 0x400);
    func_80015538(&D_801EA790 + 0x2000, 0, 0x400);
}
