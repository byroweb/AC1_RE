/* fdat034 0x801C6AE0  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s16 D_801A26C0;
extern s16 D_801A26C2;
extern s16 D_801A26C4;

s32 func_801C6AE0(void *arg0) {
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;

    temp_v0 = (s32) (M2C_FIELD(arg0, s16 *, 4) - D_801A26C0) >> 2;
    temp_v0_2 = (s32) (M2C_FIELD(arg0, s16 *, 6) - D_801A26C2) >> 2;
    temp_v0_3 = (s32) (M2C_FIELD(arg0, s16 *, 8) - D_801A26C4) >> 2;
    return (temp_v0 * temp_v0) + (temp_v0_2 * temp_v0_2) + (temp_v0_3 * temp_v0_3);
}
