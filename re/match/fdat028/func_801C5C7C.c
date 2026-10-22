#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct { char b[8]; } Blk8;
s32 func_80073C2C(s32, M2C_UNK *, s16, M2C_UNK, s32, s32, s32, s32, s32, s32, s32, M2C_UNK *);
extern void func_80015258();

s32 func_801C5C7C(void *arg0) {
    union { s8 b[0x48]; s32 w; } sp_frame;

    if (func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), (M2C_UNK *)((u8 *)arg0 + 8), M2C_FIELD(arg0, s16 *, 0x12), 0, 0, (s32) M2C_FIELD(arg0, u16 *, 0x36), 15000, -800, 200, -2048, 2048, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) {
        if ((*(u16 *)(sp_frame.b + 0x16)) & 2) {
            (*(s16 *)(sp_frame.b + 0x30)) = (*(u16 *)(sp_frame.b + 0x10)) - M2C_FIELD(arg0, u16 *, 0x108);
            (*(s16 *)(sp_frame.b + 0x32)) = (*(u16 *)(sp_frame.b + 0x12)) - M2C_FIELD(arg0, u16 *, 0x10A);
            (*(s16 *)(sp_frame.b + 0x34)) = (*(u16 *)(sp_frame.b + 0x14)) - M2C_FIELD(arg0, u16 *, 0x10C);
            *(Blk8 *)((u8 *)arg0 + 0x108) = *(Blk8 *)(sp_frame.b + 0x10);
            func_80015258((u8 *)arg0 + 8, (u8 *)arg0 + 0x10, 0x2BC, sp_frame.b + 0x10, sp_frame.b + 0x30, sp_frame.b + 0x38, sp_frame.b + 0x40, 3);
            M2C_FIELD(arg0, u16 *, 0x76) = (*(u16 *)(sp_frame.b + 0x40));
            M2C_FIELD(arg0, s16 *, 0x78) = (*(u16 *)(sp_frame.b + 0x42)) + M2C_FIELD(arg0, u16 *, 0x12);
            M2C_FIELD(arg0, s16 *, 0x7A) = 0;
            return 1;
        }
        goto block_4;
    }
block_4:
    M2C_FIELD(arg0, s16 *, 0x7A) = 0;
    M2C_FIELD(arg0, s16 *, 0x78) = 0;
    M2C_FIELD(arg0, u16 *, 0x76) = 0U;
    return 0;
}
