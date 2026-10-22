#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, M2C_UNK *);               /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32, s32); /* extern */
M2C_UNK func_80078598();                            /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C513C(void *);                          /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C7060;
extern M2C_UNK D_801C7084;

void func_801C52A0(void *arg0) {
    u8 temp_v0;
    void *temp_v0_2;

    func_80078598();
    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C7060) != 0) {
            func_80078E1C(arg0);
        }
    } else if ((M2C_FIELD(arg0, s16 *, 0x7C) == 0) && (M2C_FIELD(arg0, u8 *, 0x42) != 0) && (func_801C513C(arg0) != 0)) {
        M2C_FIELD(arg0, s16 *, 0x7C) = 0;
        if ((M2C_FIELD(arg0, s32 *, 0x3C) & 0xFFFF00) == 0xC000) {
            if (rand() < 0x51F) {
                M2C_FIELD(arg0, u8 *, 0x3E) = 5U;
            }
        } else {
            temp_v0 = M2C_FIELD(arg0, u8 *, 0x3F);
            if (temp_v0 == 0) {
                temp_v0_2 = M2C_FIELD(arg0, void **, 0);
                func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0_2, u16 *, 0x1C), M2C_FIELD(temp_v0_2, u8 *, 0x20), &D_801C7084, 1, 0, 1);
                M2C_FIELD(arg0, u8 *, 0x3F) = 1U;
                M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                M2C_FIELD(arg0, s32 *, 0x15C) = 1;
                M2C_FIELD(arg0, u8 *, 0x3E) = (u8) (M2C_FIELD(arg0, u8 *, 0x3E) - 1);
                return;
            }
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (temp_v0 - 1);
        }
    }
}
