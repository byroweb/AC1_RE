#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80073C2C(s32, M2C_UNK *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, M2C_UNK *); /* extern */
s32 func_800745D0(void *, void *, s32, s32, s32, s32);

s32 func_801C575C(void *arg0) {
    union { s8 b[0x38]; s32 w; } sp_frame;

    if (func_800745D0(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x30)), 1, 0, 1, 1) != 0) {
        if ((func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), &(*(M2C_UNK *)(sp_frame.b + 0x30)), M2C_FIELD(arg0, s16 *, 0x12), 0, 8, (s32) M2C_FIELD(arg0, u16 *, 0x36), 0x3A98, -0x258, 0x258, -0x640, 0x640, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2)) {
            M2C_FIELD(arg0, u16 *, 0x76) = (*(u16 *)(sp_frame.b + 0x18));
            M2C_FIELD(arg0, s16 *, 0x78) = (s16) -(s16) (*(u16 *)(sp_frame.b + 0x1A));
            M2C_FIELD(arg0, s16 *, 0x7A) = 0;
            return 1;
        }
        goto block_5;
    }
block_5:
    M2C_FIELD(arg0, s16 *, 0x7A) = 0;
    M2C_FIELD(arg0, s16 *, 0x78) = 0;
    M2C_FIELD(arg0, u16 *, 0x76) = 0U;
    return 0;
}
