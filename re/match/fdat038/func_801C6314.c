/* fdat038 0x801C6314  size 216  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80052338();                                /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8008A778(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8008A80C();                            /* extern */
extern u16 D_8019F528;
extern u16 D_8019F52C;
extern s32 D_801C6500;
extern s32 D_801C6504;
extern s32 D_801C6508;

void func_801C6314(void) {
    func_8008A80C();
    if (D_8019F528 == 0) {
        if (D_801C6500 == 0xDC) {
            func_800529F0(0, 0, 0x4F);
        }
        if ((D_801C6504 != 0) && (func_80052338() != 0)) {
            func_8008A778(0xF78, 0x100, 0x95);
            D_801C6504 = 0;
            D_801C6508 = 1;
        }
        if (D_8019F52C == 0x528) {
            func_800529F0(0, 0, 0x50);
        }
        if (D_801C6500 <= 0xFFFE) {
            D_801C6500 += 1;
        }
    }
}
