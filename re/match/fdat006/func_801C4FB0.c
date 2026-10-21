#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800754D0(void *, void *);                       /* extern */
M2C_UNK func_80077B78(void *, M2C_UNK, u16, u8, M2C_UNK *, s32, s32); /* extern */
M2C_UNK func_80078E1C(void *);                      /* extern */
s32 func_801C4EC8(void *);                          /* extern */
s32 rand();                                         /* extern */
extern M2C_UNK D_801C61B0;
extern M2C_UNK D_801C61D4;

void func_801C4FB0(void *arg0) {
    u8 temp_v1;
    u8 var_v0;
    void *temp_v0;

    M2C_FIELD(arg0, s16 *, 0x148) = 0;
    M2C_FIELD(arg0, s32 *, 0x15C) = 0;
    if (M2C_FIELD(arg0, u8 *, 0x40) != 0xFF) {
        if (func_800754D0(arg0, &D_801C61B0) != 0) {
            func_80078E1C(arg0);
        }
    } else {
        temp_v1 = M2C_FIELD(arg0, u8 *, 0x3F);
        if (temp_v1 != 0 && M2C_FIELD(arg0, u8 *, 0x3E) == 0) {
            M2C_FIELD(arg0, u8 *, 0x3F) = temp_v1 - 1;
        } else if ((rand() < 0x290) && ((rand() < 0xCCD) || (func_801C4EC8(arg0) != 0))) {
            temp_v0 = M2C_FIELD(arg0, void **, 0);
            func_80077B78(arg0, 0x27, M2C_FIELD(temp_v0, u16 *, 0x1C), M2C_FIELD(temp_v0, u8 *, 0x20), &D_801C61D4, 1, 0);
            M2C_FIELD(arg0, s16 *, 0x148) = 0xF;
            M2C_FIELD(arg0, u8 *, 0x14A) = 0xC8;
            M2C_FIELD(arg0, u8 *, 0x14B) = 0x3C;
            M2C_FIELD(arg0, u8 *, 0x14C) = 0;
            M2C_FIELD(arg0, s32 *, 0x15C) = 0x100;
            M2C_FIELD(arg0, u8 *, 0x3F) = 5;
        }
    }
}
