#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, M2C_UNK *);               /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32, s32, s32, s32, s32); /* extern */
M2C_UNK func_80078598();                            /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C575C(void *);                          /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801CC9F4;
extern M2C_UNK D_801CCA18;

void func_801C5850(void *arg0) {
    void *temp_v0;

    func_80078598();
    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801CC9F4) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        if (func_801C575C(arg0) != 0) {
            if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
                temp_v0 = M2C_FIELD(arg0, void **, 0);
                func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0, u16 *, 0x1C), M2C_FIELD(temp_v0, u8 *, 0x20), &D_801CCA18, 1, 0, 1, 0, 0, M2C_FIELD(arg0, s32 *, 0x48));
                M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                M2C_FIELD(arg0, u8 *, 0x14A) = 0xC8;
                M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                M2C_FIELD(arg0, s32 *, 0x15C) = 1;
                M2C_FIELD(arg0, u8 *, 0x3F) = (u8) ((rand() & 0x1F) + 0x32);
                goto block_6;
            }
            goto block_7;
        }
block_6:
        if (M2C_FIELD(arg0, u8 *, 0x3F) != 0) {
block_7:
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (M2C_FIELD(arg0, u8 *, 0x3F) - 1);
        }
    }
}
