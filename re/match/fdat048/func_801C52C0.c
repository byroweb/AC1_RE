/* fdat048 0x801C52C0  size 128  sweep.py best draft: ctx, closeness 0.719 aligned / 0.219 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
void func_801CB4D8(void *);
extern M2C_UNK D_801CB524;
extern M2C_UNK func_801C9648;
extern M2C_UNK func_801C9B58;

void func_801C52C0(void *arg0) {
    if (M2C_FIELD(arg0, u8 *, 0x100) == 1) {
        M2C_FIELD(arg0, s16 *, 0x36) = 1;
        M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C9648;
        M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C9B58;
        func_8004E98C(arg0 + 0x64, 0, 0, 0);
        return;
    }
    M2C_FIELD(arg0, s16 *, 0x36) = 0x102;
    M2C_FIELD(arg0, M2C_UNK **, 0x108) = &D_801CB524;
    func_801CB4D8(arg0);
}
