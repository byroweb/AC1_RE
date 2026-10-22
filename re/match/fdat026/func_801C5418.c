/* fdat026 0x801C5418  size 132  permuter tier (tools/loop/permute.py): levers bytes E + &D_8019FAB8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK D_8019FAB8;
extern s16 D_801A26C0;
extern s16 D_801A26C2;
extern s16 D_801A26C4;

s32 func_801C5418(s32 arg0) {
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    void *temp_v1;

    temp_v1 = ((M2C_UNK *)((u8 *)&D_8019FAB8 + (arg0 * 0x2C)));
    temp_v0 = (s32) (M2C_FIELD(temp_v1, s16 *, 4) - D_801A26C0) >> 2;
    temp_v0_2 = (s32) (M2C_FIELD(temp_v1, s16 *, 6) - D_801A26C2) >> 2;
    temp_v0_3 = (s32) (M2C_FIELD(temp_v1, s16 *, 8) - D_801A26C4) >> 2;
    return (temp_v0 * temp_v0) + (temp_v0_2 * temp_v0_2) + (temp_v0_3 * temp_v0_3);
}
