/* fdat024 0x801C6CCC  size 200  permuter tier (tools/loop/permute.py): levers bytes E + &D_80198B08, from the ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78(M2C_UNK);                         /* extern */
void *func_80086F20(M2C_UNK, s16 *, M2C_UNK, M2C_UNK, s32, M2C_UNK *, M2C_UNK *, M2C_UNK *, M2C_UNK *); /* extern */
extern M2C_UNK D_80198B08;
extern M2C_UNK D_801C70D4;
extern M2C_UNK D_801C70DC;
extern M2C_UNK D_801C70E4;
extern M2C_UNK D_801C70E8;
extern M2C_UNK func_801C6C5C;

void func_801C6CCC(s32 arg0) {
    union { s8 b[0x8]; s32 w; } sp_frame;
    void *temp_s0;
    void *temp_v0;

    temp_s0 = ((M2C_UNK *)((u8 *)&D_80198B08 + (arg0 * 0x34)));
    (*(s16 *)(sp_frame.b + 0x0)) = (M2C_FIELD(temp_s0, u16 *, 0) + func_80014D78(0x3E8)) - 0x1F4;
    (*(u16 *)(sp_frame.b + 0x2)) = M2C_FIELD(temp_s0, u16 *, 2);
    (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(temp_s0, u16 *, 4);
    temp_v0 = func_80086F20(0, &(*(s16 *)(sp_frame.b + 0x0)), 8, 0x15E, 0, &D_801C70D4, &D_801C70DC, &D_801C70E4, &D_801C70E8);
    M2C_FIELD(temp_v0, M2C_UNK **, 0x14) = &func_801C6C5C;
    M2C_FIELD(temp_v0, s16 *, 0x12) = 0xB;
}
