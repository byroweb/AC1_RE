/* fdat020 0x801C5418  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008A80C();                            /* extern */
M2C_UNK func_8008C2FC(M2C_UNK);                     /* extern */
extern s32 D_801CAF80;

void func_801C5418(void) {
    if (D_801CAF80 == 0) {
        func_8008C2FC(0x601);
        D_801CAF80 = 1;
    }
    func_8008A80C();
}
