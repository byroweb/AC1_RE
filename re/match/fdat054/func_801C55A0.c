/* fdat054 0x801C55A0  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008A80C();                            /* extern */
M2C_UNK func_8008C2FC(M2C_UNK);                     /* extern */
extern s32 D_801CAF60;

void func_801C55A0(void) {
    if (D_801CAF60 == 0) {
        func_8008C2FC(0x600);
        D_801CAF60 = 1;
    }
    func_8008A80C();
}
