/* fdat164 0x801C8DCC  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u16 D_80040FB0;
extern void *D_80041338;
extern u16 D_8004134C;
extern void *D_800416D4;

void func_801C8DCC(void) {
    if ((D_80040FB0 & 7) == 1) {
        M2C_FIELD(D_80041338, u16 *, 8) = (u16) (M2C_FIELD(D_80041338, u16 *, 8) | 1);
    }
    if ((D_8004134C & 7) == 1) {
        M2C_FIELD(D_800416D4, u16 *, 8) = (u16) (M2C_FIELD(D_800416D4, u16 *, 8) | 1);
    }
}
