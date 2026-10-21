/* fdat022 0x801C6758  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK func_801C60B0;
extern M2C_UNK func_801C66C4;

void func_801C6758(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 2;
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C60B0;
    M2C_FIELD(arg0, s32 *, 0x50) = 0;
    M2C_FIELD(arg0, s32 *, 0x54) = 0;
    M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C66C4;
    M2C_FIELD(arg0, s8 *, 0x42) = 0;
    M2C_FIELD(arg0, s16 *, 0x44) = 0;
    func_8004E98C(arg0 + 0x64, 0, 0, 0);
    M2C_FIELD(arg0, s8 *, 0x10B) = 0;
    M2C_FIELD(arg0, s8 *, 0x10A) = 0;
}
