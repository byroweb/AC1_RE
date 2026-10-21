#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 b[8];
} Bytes8_801C5A90;
extern Bytes8_801C5A90 D_80039C78;

s32 func_80073C2C(s32, void *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, M2C_UNK *); /* extern */

s32 func_801C5A90(void *arg0) {
    union { s8 b[0x30]; s32 w; } sp_frame;

    if ((func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), arg0 + 8, M2C_FIELD(arg0, s16 *, 0x12), 0, 0, (s32) M2C_FIELD(arg0, u16 *, 0x36), (s32) M2C_FIELD(arg0, u16 *, 0xFC), -0x400, 0x400, -0x800, 0x800, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2)) {
        M2C_FIELD(arg0, u16 *, 0x76) = (*(u16 *)(sp_frame.b + 0x18));
        M2C_FIELD(arg0, s16 *, 0x78) = (s16) ((*(u16 *)(sp_frame.b + 0x1A)) + (u16) M2C_FIELD(arg0, s16 *, 0x12));
        M2C_FIELD(arg0, s16 *, 0x7A) = 0;
        return 1;
    }
    M2C_FIELD(arg0, Bytes8_801C5A90 *, 0x76) = D_80039C78;
    return 0;
}
