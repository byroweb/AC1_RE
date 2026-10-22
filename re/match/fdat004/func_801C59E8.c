#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_80073904();                                /* extern */
M2C_UNK func_801C4FFC(void *, u8);                  /* extern */
M2C_UNK func_801C5534(void *, M2C_UNK);             /* extern */
extern M2C_UNK func_801C52A0;
extern M2C_UNK func_801C58AC;

void func_801C59E8(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);
    M2C_UNK (*temp_v1)(void *, M2C_UNK);

    M2C_FIELD(arg0, s16 *, 0x36) = 2;
    M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50) = func_801C4FFC;
    M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK), 0x54) = func_801C5534;
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C52A0;
    M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C58AC;
    M2C_FIELD(arg0, s8 *, 0x42) = 0;
    M2C_FIELD(arg0, u16 *, 0x164) = (u16) (M2C_FIELD(arg0, u16 *, 0x164) * 4);
    M2C_FIELD(arg0, s16 *, 0x44) = func_80073904();
    temp_v1 = M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK), 0x54);
    if (temp_v1 != NULL) {
        temp_v1(arg0, 0);
    }
    temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
    if (temp_v0 != NULL) {
        temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
    }
}
