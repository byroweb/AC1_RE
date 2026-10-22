#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 b[8];
} Bytes8_801C5C40;
extern Bytes8_801C5C40 D_80039C78;

s16 func_800132CC(s16, s16, M2C_UNK);               /* extern */
s32 func_80073C2C(s32, M2C_UNK *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, M2C_UNK *); /* extern */
s32 func_800745D0(void *, void *, s32, s32, s32, s32);

s32 func_801C5C40(void *arg0) {
    union { s8 b[0x38]; s32 w; } sp_frame;

    if ((func_800745D0(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x30)), 1, 5, 1, 1) != 0) && (func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), &(*(M2C_UNK *)(sp_frame.b + 0x30)), M2C_FIELD(arg0, s16 *, 0x12), 0, 0, (s32) M2C_FIELD(arg0, u16 *, 0x36), 0x2710, -0x100, 0x400, -0x200, 0x200, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2)) {
        M2C_FIELD(arg0, s16 *, 0x76) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x76), (*(s16 *)(sp_frame.b + 0x18)), 0x18);
        M2C_FIELD(arg0, s16 *, 0x7C) = 0;
        return 1;
    }
    M2C_FIELD(arg0, Bytes8_801C5C40 *, 0x76) = D_80039C78;
    return 0;
}
