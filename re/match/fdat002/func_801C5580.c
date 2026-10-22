/* fdat002 0x801C5580  size 296  permuter tier (tools/loop/permute.py): levers args func_800745D0 +arg0 proto, from the ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_800132CC(s16, s16, M2C_UNK);               /* extern */
s32 func_80073C2C(s32, M2C_UNK *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, M2C_UNK *); /* extern */
s32 func_800745D0(void *, M2C_UNK *, u8, u16, s32, s32);    /* extern */

s32 func_801C5580(void *arg0, void *arg1) {
    union { s8 b[0x38]; s32 w; } sp_frame;

    if (func_800745D0(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x30)), M2C_FIELD(arg1, u8 *, 0), M2C_FIELD(arg1, u16 *, 2), 1, 1) != 0) {
        if ((func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), &(*(M2C_UNK *)(sp_frame.b + 0x30)), M2C_FIELD(arg0, s16 *, 0x12), 0, 0, (s32) M2C_FIELD(arg0, u16 *, 0x36), 0x1B58, -0x400, 0x200, -0xFA, 0xFA, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2)) {
            M2C_FIELD(arg0, s16 *, 0x76) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x76), (*(s16 *)(sp_frame.b + 0x18)), 0x10);
            M2C_FIELD(arg0, s16 *, 0x78) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x78), (s16) ((s32) (*(u16 *)(sp_frame.b + 0x1A)) * -1), 0x10);
            return 1;
        }
        goto block_5;
    }
block_5:
    M2C_FIELD(arg0, s16 *, 0x76) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x76), 0, 0x10);
    M2C_FIELD(arg0, s16 *, 0x78) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x78), 0, 0x10);
    return 0;
}
