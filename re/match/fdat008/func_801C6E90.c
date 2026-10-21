/* fdat008 0x801C6E90  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004C318(M2C_UNK);                     /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8008C2FC(M2C_UNK);                     /* extern */
extern u16 D_8019F528;
extern u8 D_8019FB94;

s32 func_801C6E90(s32 arg0) {
    if ((D_8019F528 == 0) && (arg0 == 0)) {
        if (D_8019FB94 == 1) {
            func_8008C2FC(0);
        }
        func_800529F0(0, 0, 0x68);
        func_8004C318(0x80);
    }
    return 1;
}
