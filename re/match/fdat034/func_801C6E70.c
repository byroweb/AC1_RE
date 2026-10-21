/* fdat034 0x801C6E70  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004C318(M2C_UNK);                     /* extern */
s32 func_80052338();                                /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8008A80C();                            /* extern */
extern u16 D_8019F528;
extern s32 D_801CCA80;
extern s32 D_801CCA84;

void func_801C6E70(void) {
    func_8008A80C();
    if ((D_8019F528 == 0) && (D_801CCA80 != 0) && (func_80052338() != 0)) {
        if (D_801CCA84 == 0xA) {
            func_800529F0(0, 0, 0x68);
            func_8004C318(0x80);
        }
        D_801CCA84 += 1;
    }
}
