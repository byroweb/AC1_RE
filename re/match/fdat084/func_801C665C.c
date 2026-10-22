#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, void *);
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32, s32); /* extern */
s32 func_800781E8(void *);                          /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C656C(void *);                          /* extern */
extern M2C_UNK D_801C7970;
extern M2C_UNK D_801C7994;

void func_801C665C(void *arg0) {
    void *temp_v0;

    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C7970) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        if ((M2C_FIELD(arg0, s16 *, 0x7C) == 0) && (func_801C656C(arg0) != 0)) {
            if (M2C_FIELD(arg0, u8 *, 0x3F) == 0) {
                if (func_800781E8(arg0) != 0) {
                    temp_v0 = M2C_FIELD(arg0, void **, 0);
                    func_80077B78(arg0, 0x17, M2C_FIELD(temp_v0, u16 *, 0x1C), M2C_FIELD(temp_v0, u8 *, 0x20), &D_801C7994, 1, 0, 1);
                    M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
                    M2C_FIELD(arg0, u8 *, 0x14A) = 0xB4;
                    M2C_FIELD(arg0, u8 *, 0x14B) = 0xA0;
                    M2C_FIELD(arg0, u8 *, 0x14C) = 0x8C;
                    M2C_FIELD(arg0, s32 *, 0x15C) = 1;
                    M2C_FIELD(arg0, u8 *, 0x3F) = 0x1EU;
                }
                goto block_8;
            }
            goto block_9;
        }
block_8:
        if (M2C_FIELD(arg0, u8 *, 0x3F) != 0) {
block_9:
            M2C_FIELD(arg0, u8 *, 0x3F) = (u8) (M2C_FIELD(arg0, u8 *, 0x3F) - 1);
        }
    }
}
