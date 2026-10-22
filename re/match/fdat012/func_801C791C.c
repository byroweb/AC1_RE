/* fdat012 0x801C791C  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern u16 D_8019F528;
extern s32 D_801C7DE4;

s32 func_801C791C(s32 arg0) {
    if ((D_8019F528 == 0) && (arg0 == 0)) {
        func_800529F0(0, 0, 0x6A);
        D_801C7DE4 = 1;
    }
    return 1;
}
