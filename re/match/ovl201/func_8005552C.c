/* ovl201 0x8005552C  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8004FDF4;
extern M2C_UNK func_800A2D0C;

void func_8005552C(void *arg0) {
    s32 temp_a1;
    void *temp_v1;

    temp_v1 = M2C_FIELD(arg0, void **, 0x7C);
    temp_a1 = M2C_FIELD(temp_v1, s32 *, 0x70);
    if (temp_a1 == &func_800A2D0C) {
        M2C_FIELD(temp_v1, s8 *, 0x9C) = 0x4C;
    } else if (temp_a1 == &func_8004FDF4) {
        M2C_FIELD(temp_v1, s8 *, 0x7D) = 1;
    }
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
    M2C_FIELD(arg0, s8 *, 1) = 1;
}
