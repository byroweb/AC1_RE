#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80076578(void *, M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
M2C_UNK func_8007790C(void *, u8, M2C_UNK, M2C_UNK, void *, s32, M2C_UNK *); /* extern */
M2C_UNK func_80078598(void *);                      /* extern */
s32 func_801C5B8C(void *);                          /* extern */
M2C_UNK func_801C5C68(void *, M2C_UNK, M2C_UNK);    /* extern */
extern M2C_UNK D_800764C4;
extern M2C_UNK func_801C5910;

void func_801C5DBC(void *arg0, s32 arg1) {
    s32 var_s1;

    var_s1 = arg1;
    if (M2C_FIELD(arg0, u16 *, 0x36) & 0x10) {
        var_s1 = M2C_FIELD(arg0, u8 *, 0x3C);
    }
    func_80078598(arg0);
    switch (var_s1) {                               /* irregular */
    case 1:
        func_801C5C68(arg0, 2, 1);
        break;
    case 2:
        func_801C5C68(arg0, 5, 4);
        break;
    case 0:
        if (func_801C5B8C(arg0) != 0) {
            func_80076578(arg0, 0, 1, &func_801C5910);
            M2C_FIELD(arg0, s16 *, 0x72) = 0x100;
        }
        break;
    case 3:
        func_8007790C(arg0, M2C_FIELD(arg0, u8 *, 0x64), 1, 0x400, arg0 + 0xCC, -1, &D_800764C4);
        break;
    }
    if (!(M2C_FIELD(arg0, u16 *, 0x36) & 0x10)) {
        M2C_FIELD(arg0, u8 *, 0x3C) = var_s1;
    }
}
