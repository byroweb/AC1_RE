/* fdat002 0x801C6BE4  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_80078CFC(M2C_UNK *);                   /* extern */
extern M2C_UNK D_8019FE28;

s32 func_801C6BE4(s32 arg0) {
    if (arg0 == 0) {
        func_800529F0(0, 0, 1);
        func_80078CFC(&D_8019FE28);
    }
    return 1;
}
