/* fdat012 0x801C715C  size 320  sweep.py best draft: ctx-spframe, closeness 0.863 aligned / 0.850 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u32 D_801AC860[];

M2C_UNK func_80073170();                            /* extern */
M2C_UNK func_80086854(M2C_UNK, u16 *, M2C_UNK *);   /* extern */
M2C_UNK func_800873F8(void *, u16 *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
s32 func_8008A0B0(u8, u8, u16, u8, s32);            /* extern */
extern M2C_UNK D_801C7DE0;

s32 func_801C715C(void *arg0, s32 arg1) {
    union { s8 b[0x8]; s32 w; } sp_frame;
    u16 temp_a0;

    if (arg1 == 0) {
        if (func_8008A0B0(M2C_FIELD(arg0, u8 *, 0x22), M2C_FIELD(arg0, u8 *, 0x23), M2C_FIELD(arg0, u16 *, 0x24), M2C_FIELD(arg0, u8 *, 0x26), (s32) M2C_FIELD(arg0, u8 *, 0x27)) != 0) {
            if (M2C_FIELD(arg0, u8 *, 0x28) != 0) {
                M2C_FIELD(arg0, s16 *, 6) = -1;
                M2C_FIELD(arg0, s32 *, 0x14) = 0;
                goto block_8;
            }
        }
    } else if (M2C_FIELD(arg0, u8 *, 0x28) == 2) {
        func_8008A0B0(M2C_FIELD(arg0, u8 *, 0x2A), M2C_FIELD(arg0, u8 *, 0x2B), M2C_FIELD(arg0, u16 *, 0x2C), M2C_FIELD(arg0, u8 *, 0x2E), (s32) M2C_FIELD(arg0, u8 *, 0x2F));
        temp_a0 = M2C_FIELD(arg0, u16 *, 0xE);
        if (D_801AC860[(s16)temp_a0 >> 5] & (1 << (temp_a0 & 0x1F))) {
            (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 0);
            (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 4);
            (*(s16 *)(sp_frame.b + 0x2)) = M2C_FIELD(arg0, u16 *, 2) - ((u16) M2C_FIELD(arg0, u16 *, 0x20) >> 1);
            func_80086854(0, &(*(u16 *)(sp_frame.b + 0x0)), &D_801C7DE0);
            func_800873F8(arg0, &(*(u16 *)(sp_frame.b + 0x0)), 0xA, 0x14, 8, 0);
        }
        M2C_FIELD(arg0, s16 *, 6) = -1;
        M2C_FIELD(arg0, s32 *, 0x14) = 0;
        M2C_FIELD(arg0, u16 *, 0x1C) = (u16) (M2C_FIELD(arg0, u16 *, 0x1C) | 0xC000);
block_8:
        func_80073170();
    }
    return 1;
}
