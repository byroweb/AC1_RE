#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 b[8];
} Bytes8_801C51B8;
extern Bytes8_801C51B8 D_80039C78;

s32 func_80073C2C(s32, void *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, s32 *); /* extern */

s32 func_801C51B8(void *arg0) {
    union { s8 b[0x30]; s32 w; } sp_frame;

    if ((func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), arg0 + 8, M2C_FIELD(arg0, s16 *, 0x12), 0, 8, (s32) M2C_FIELD(arg0, u16 *, 0x36), (s32) M2C_FIELD(arg0, u16 *, 0xFC), -0x400, 0x400, -0x800, 0x800, &(*(s32 *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2)) {
        if ((*(s16 *)(sp_frame.b + 0x18)) >= -0x3F) {
            (*(s16 *)(sp_frame.b + 0x18)) = -0x40;
        }
        M2C_FIELD(arg0, u16 *, 0x76) = (u16) (*(s16 *)(sp_frame.b + 0x18));
        M2C_FIELD(arg0, s16 *, 0x78) = (s16) ((*(u16 *)(sp_frame.b + 0x1A)) + (u16) M2C_FIELD(arg0, s16 *, 0x12));
        M2C_FIELD(arg0, s16 *, 0x7A) = 0;
        return (*(s32 *)(sp_frame.b + 0x0));
    }
    M2C_FIELD(arg0, Bytes8_801C51B8 *, 0x76) = D_80039C78;
    return 0;
}
