#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_800132CC(s16, s16, M2C_UNK);               /* extern */
s32 func_800148E4(s16, s16, M2C_UNK);               /* extern */
s32 func_800754D0(void *, void *);
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32, s32); /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C5C7C(void *);                          /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C7430;
extern M2C_UNK D_801C7454;

void func_801C5DCC(void *arg0) {
    s16 temp_v0;
    u16 temp_s0;
    u8 temp_v0_2;
    u8 temp_v0_4;
    void *temp_v0_3;

    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C7430) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        if (func_801C5C7C(arg0) != 0) {
            temp_v0 = func_800132CC(M2C_FIELD(arg0, s16 *, 0x12), M2C_FIELD(arg0, s16 *, 0x78), 0x40);
            M2C_FIELD(arg0, s16 *, 0x12) = temp_v0;
            if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
                if (M2C_FIELD(arg0, s16 *, 0x7C) == 0) {
                    temp_v0_2 = M2C_FIELD(arg0, u8 *, 0x3E);
                    if (temp_v0_2 == 0) {
                        if ((func_800148E4(temp_v0, M2C_FIELD(arg0, s16 *, 0x78), 0x20) != 0) && (rand() < 0x199A)) {
                            M2C_FIELD(arg0, u8 *, 0x3E) = 0xCU;
                        }
                    } else {
                        if (temp_v0_2 & 1) {
                            temp_s0 = (u16) M2C_FIELD(arg0, s16 *, 0x78);
                            temp_v0_3 = M2C_FIELD(arg0, void **, 0);
                            M2C_FIELD(arg0, s16 *, 0x78) = 0;
                            func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0_3, u16 *, 0x1C), M2C_FIELD(temp_v0_3, u8 *, 0x20), &D_801C7454, 1, 0, 1);
                            M2C_FIELD(arg0, s16 *, 0x78) = (s16) temp_s0;
                        }
                        temp_v0_4 = M2C_FIELD(arg0, u8 *, 0x3E) - 1;
                        M2C_FIELD(arg0, u8 *, 0x3E) = temp_v0_4;
                        if (!(temp_v0_4 & 0xFF)) {
                            M2C_FIELD(arg0, u8 *, 0x3F) = 0x2CU;
                        }
                    }
                }
                goto block_14;
            }
            goto block_15;
        }
block_14:
        if (M2C_FIELD(arg0, u8 *, 0x3F) != 0) {
block_15:
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (M2C_FIELD(arg0, u8 *, 0x3F) - 1);
        }
    }
}
