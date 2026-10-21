#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801C4EE8(void *, u8);                  /* extern */
M2C_UNK func_801C539C(M2C_UNK);                     /* extern */
extern M2C_UNK func_801C5158;
extern M2C_UNK func_801C550C;

void func_801C5560(void *arg0) {
    M2C_UNK (*temp_v0)(void *, u8);
    M2C_UNK (*temp_v1)(void *, M2C_UNK);

    M2C_FIELD(arg0, s16 *, 0x36) = 0x102;
    M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50) = func_801C4EE8;
    M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK), 0x54) = func_801C539C;
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C5158;
    M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C550C;
    temp_v1 = M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK), 0x54);
    M2C_FIELD(arg0, s16 *, 0x7C) = 0;
    M2C_FIELD(arg0, s16 *, 0x108) = 0;
    M2C_FIELD(arg0, u8 *, 0x41) = 0U;
    M2C_FIELD(arg0, u16 *, 0x164) = (u16) (M2C_FIELD(arg0, u16 *, 0x164) * 4);
    if (temp_v1 != NULL) {
        temp_v1(arg0, 0);
    }
    temp_v0 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
    if (temp_v0 != NULL) {
        temp_v0(arg0, M2C_FIELD(arg0, u8 *, 0x41));
    }
}
