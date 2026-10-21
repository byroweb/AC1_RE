#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_800132CC(s16, s16, M2C_UNK);               /* extern */
s32 func_800148E4(s16, s16, M2C_UNK);               /* extern */
s32 func_800754D0(void *, void *);                       /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, u16 *, s32, s32, s32); /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C4E14(void *);                          /* extern */
extern M2C_UNK D_801C80D8;
extern u16 D_801C8102[];

void func_801C4EF0(void *arg0) {
    u16 temp_s1;
    void *temp_v1;

    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C80D8) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        if ((M2C_FIELD(arg0, s16 *, 0x7C) == 0) && (func_801C4E14(arg0) != 0)) {
            M2C_FIELD(arg0, s16 *, 0x12) = func_800132CC(M2C_FIELD(arg0, s16 *, 0x12), M2C_FIELD(arg0, s16 *, 0x78), 0x30);
            M2C_FIELD(arg0, s16 *, 0x7C) = 0;
            if (func_800148E4(M2C_FIELD(arg0, s16 *, 0x12), M2C_FIELD(arg0, s16 *, 0x78), 0x20) != 0) {
                if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
                    D_801C8102[0] = M2C_FIELD(arg0, u16 *, 0xFC);
                    temp_s1 = (u16) M2C_FIELD(arg0, s16 *, 0x78);
                    temp_v1 = M2C_FIELD(arg0, void **, 0);
                    M2C_FIELD(arg0, s16 *, 0x78) = 0;
                    func_80077B78(arg0, 0x27, M2C_FIELD(temp_v1, u16 *, 0x1C), M2C_FIELD(temp_v1, u8 *, 0x20), &D_801C8102[-3], 1, 0, 1);
                    M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                    M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                    M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                    M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                    M2C_FIELD(arg0, s16 *, 0x78) = (s16) temp_s1;
                    M2C_FIELD(arg0, s32 *, 0x15C) = 1;
                    M2C_FIELD(arg0, u8 *, 0x3F) = 0x3CU;
                    goto block_8;
                }
                goto block_9;
            }
        }
block_8:
        if (M2C_FIELD(arg0, u8 *, 0x3F) != 0) {
block_9:
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (M2C_FIELD(arg0, u8 *, 0x3F) - 1);
        }
    }
}
