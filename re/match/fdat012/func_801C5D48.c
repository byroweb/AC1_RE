#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK LoadAverageCol(M2C_UNK *, M2C_UNK *, s32, s32, void *); /* extern */
M2C_UNK func_80056B54(void *, u16 *, M2C_UNK, s16, s32, void *, s32); /* extern */
s32 func_80073714(void *, M2C_UNK);                 /* extern */
s32 func_800754D0(void *, M2C_UNK *);               /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32, s32); /* extern */
M2C_UNK func_80078598();                            /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C5C40(void *);                          /* extern */
s32 rand();                                         /* extern */
s32 rsin(u16);                                      /* extern */
extern M2C_UNK D_801C7C5C;
extern M2C_UNK D_801C7C60;
extern M2C_UNK D_801C7C64;
extern M2C_UNK D_801C7C88;

void func_801C5D48(void *arg0) {
    union { s8 b[0x8]; s32 w; } sp_frame;
    s32 temp_a2;
    s32 temp_a3;
    u32 temp_v1;
    u8 temp_s1_2;
    u8 temp_v0;
    void *temp_s1;
    void *temp_v0_2;

    func_80078598();
    M2C_FIELD(arg0, s16 *, 0x148) = 0xF0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    temp_a2 = (s32) (rsin(M2C_FIELD(arg0, u16 *, 0xFC)) + 0x1000) >> 1;
    temp_a3 = 0x1000 - temp_a2;
    temp_s1 = arg0 + 0x14E;
    M2C_FIELD(arg0, u16 *, 0xFC) = (u16) (M2C_FIELD(arg0, u16 *, 0xFC) + 0x40);
    LoadAverageCol(&D_801C7C5C, &D_801C7C60, temp_a2, temp_a3, temp_s1);
    temp_v1 = M2C_FIELD(arg0, s16 *, 0x2C) - M2C_FIELD(arg0, s16 *, 0xA);
    if (temp_v1 < 0x1001U) {
        (*(u16 *)(sp_frame.b + 0x0)) = M2C_FIELD(arg0, u16 *, 8);
        (*(u16 *)(sp_frame.b + 0x4)) = M2C_FIELD(arg0, u16 *, 0xC);
        (*(u16 *)(sp_frame.b + 0x2)) = (u16) M2C_FIELD(arg0, s16 *, 0x2C);
        func_80056B54(arg0, &(*(u16 *)(sp_frame.b + 0x0)), 0, M2C_FIELD(arg0, s16 *, 0x16), 0xFFFF, temp_s1, ((s32) temp_v1 >> 2) + 0x400);
    }
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C7C64) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        temp_s1_2 = M2C_FIELD(arg0, u8 *, 0x42);
        if (temp_s1_2 == 1) {
            if (func_801C5C40(arg0) != 0) {
                if ((M2C_FIELD(arg0, s32 *, 0x3C) & 0xFFFF00) == 0xC000) {
                    if ((func_80073714(arg0, 0x7FFFFFFF) < 0x5DC) && (rand() < 0xCCD)) {
                        M2C_FIELD(arg0, u8 *, 0x3E) = 5U;
                    }
                } else {
                    temp_v0 = M2C_FIELD(arg0, u8 *, 0x3F);
                    if (temp_v0 == 0) {
                        temp_v0_2 = M2C_FIELD(arg0, void **, 0);
                        func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0_2, u16 *, 0x1C), M2C_FIELD(temp_v0_2, u8 *, 0x20), &D_801C7C88, (s32) temp_s1_2, 0, (s32) temp_s1_2);
                        M2C_FIELD(arg0, u8 *, 0x3F) = 1U;
                        M2C_FIELD(arg0, s16 *, 0x148) = 0xFF;
                        M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                        M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                        M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                        M2C_FIELD(arg0, s32 *, 0x15C) = 0x20;
                        M2C_FIELD(arg0, u8 *, 0x3E) = (u8) (M2C_FIELD(arg0, u8 *, 0x3E) - 1);
                        return;
                    }
                    M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (temp_v0 - 1);
                }
            }
        } else {
            M2C_FIELD(arg0, s16 *, 0x7C) = -1;
        }
    }
}
