#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80073C2C(); /* extern */
s32 func_800745D0(); /* extern */
s32 func_800754D0();                       /* extern */
M2C_UNK func_80077B78(); /* extern */
s32 func_800781E8();                          /* extern */
M2C_UNK func_80078E1C();                      /* extern */
s32 func_801C5580();               /* extern */
s32 rand();                                   /* extern */
extern u8 D_801C6D10[][32];
extern M2C_UNK D_801C6D90;
extern M2C_UNK D_801C6DB4;

void func_801C56A8(void *arg0) {
    union { s8 b[0x38]; s32 w; } sp_frame;
    M2C_UNK *temp_s1;
    s32 var_a1;
    u8 temp_s2;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 temp_v1;
    u8 var_v0;
    void *temp_v0;
    void *temp_v0_4;

    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C6D90) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        temp_s2 = M2C_FIELD(arg0, u8 *, 0xFC);
        if ((temp_s2 == 1) && ((rand() < 0x148) || ((M2C_FIELD(M2C_FIELD(arg0, void **, 0), u8 *, 0x24) != 0) && (rand() < 0x3D8))) && (func_800745D0(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x30)), 0, 0xA, (s32) temp_s2, (s32) temp_s2) != 0) && (func_80073C2C(M2C_FIELD(arg0, s32 *, 0x48), &(*(M2C_UNK *)(sp_frame.b + 0x30)), M2C_FIELD(arg0, s16 *, 0x12), 0, 8, (s32) M2C_FIELD(arg0, u16 *, 0x36), 0x2710, -0x320, 0x320, -0x1F4, 0x1F4, &(*(M2C_UNK *)(sp_frame.b + 0x0))) != 0) && ((*(u16 *)(sp_frame.b + 0x16)) & 2) && (func_800781E8(arg0) != 0)) {
            temp_v0 = M2C_FIELD(arg0, void **, 0);
            func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0, u16 *, 0x1E), M2C_FIELD(temp_v0, u8 *, 0x21), &D_801C6DB4, (s32) temp_s2, 0, (s32) temp_s2, 0, 0, M2C_FIELD(arg0, s32 *, 0x48));
        }
        if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
            temp_s1 = D_801C6D10[M2C_FIELD(arg0, u8 *, 0xFC)];
            if (func_801C5580(arg0, temp_s1) != 0) {
                temp_v0_3 = M2C_FIELD(arg0, u8 *, 0x3E);
                if (temp_v0_3 == 0) {
                    if ((rand() < 0xA3E) && ((rand() < 0x148) || (func_800781E8(arg0) != 0))) {
                        M2C_FIELD(arg0, u8 *, 0x3E) = 6U;
                    }
                } else {
                    if (temp_v0_3 & 1) {
                        var_a1 = 0x10;
                        if (M2C_FIELD(arg0, u8 *, 0xFC) != 2) {
                            var_a1 = 0x20;
                        }
                        temp_v0_4 = M2C_FIELD(arg0, void **, 0);
                        func_80077B78(arg0, var_a1 | 7, M2C_FIELD(temp_v0_4, u16 *, 0x1C), M2C_FIELD(temp_v0_4, u8 *, 0x20), temp_s1, 1, (s32) M2C_FIELD(arg0, u8 *, 0x108), M2C_FIELD(arg0, u8 *, 0x108) + 1);
                        M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                        M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                        M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                        M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                        M2C_FIELD(arg0, s32 *, 0x15C) = 0x10;
                        M2C_FIELD(arg0, u8 *, 0x108) = (u8) (M2C_FIELD(arg0, u8 *, 0x108) ^ 2);
                    }
                    temp_v1 = M2C_FIELD(arg0, u8 *, 0x3E);
                    M2C_FIELD(arg0, u8 *, 0x3E) = (u8) (temp_v1 + 0xFF);
                    if (temp_v1 == 0) {
                        M2C_FIELD(arg0, u8 *, 0x3F) = 0xA;
                    }
                }
            }
        } else {
            M2C_FIELD(arg0, u8 *, 0x3F) -= 1;
        }
    }
}
