/* ovl202 0x800718B4  size 80  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800718B4(void *arg0) {
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_v1;
    s32 temp_v1_2;

    temp_a1 = M2C_FIELD(arg0, s32 *, 0);
    temp_v1 = M2C_FIELD(arg0, s32 *, 0xC);
    if (temp_v1 < temp_a1) {
        M2C_FIELD(arg0, s32 *, 0) = temp_v1;
        M2C_FIELD(arg0, s32 *, 0xC) = (s32) (s16) temp_a1;
    }
    temp_a1_2 = M2C_FIELD(arg0, s32 *, 8);
    temp_v1_2 = M2C_FIELD(arg0, s32 *, 0x14);
    if (temp_v1_2 < temp_a1_2) {
        M2C_FIELD(arg0, s32 *, 8) = temp_v1_2;
        M2C_FIELD(arg0, s32 *, 0x14) = (s32) (s16) temp_a1_2;
    }
}
