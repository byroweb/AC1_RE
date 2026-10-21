/* fdat024 0x801C6C1C  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004CE44(M2C_UNK, void *, M2C_UNK, M2C_UNK); /* extern */

void func_801C6C1C(void *arg0) {
    if (!(M2C_FIELD(arg0, s32 *, 0x30) & 0x1000)) {
        func_8004CE44(0x4E, arg0 + 0x10, 0x64, 0);
    }
}
