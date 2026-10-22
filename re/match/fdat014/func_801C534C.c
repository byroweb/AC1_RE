/* fdat014 0x801C534C  size 380  sweep.py best draft: ctx, closeness 0.926 aligned / 0.126 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004EBF8(void *);                          /* extern */
M2C_UNK func_80076578(void *, M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
M2C_UNK func_800765E8(void *, M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
M2C_UNK func_8007790C(void *, u8, M2C_UNK, M2C_UNK, void *, s32, M2C_UNK *); /* extern */
M2C_UNK func_80078598(void *);                      /* extern */
s32 func_801C50F4(void *);                          /* extern */
M2C_UNK func_801C51F8(void *, M2C_UNK, M2C_UNK);    /* extern */
M2C_UNK func_801C52E8(void *, M2C_UNK);             /* extern */
extern M2C_UNK D_800764C4;
extern M2C_UNK func_801C4D98;
extern M2C_UNK func_801C4E00;

void func_801C534C(void *arg0, s32 arg1) {
    s32 var_s1;

    var_s1 = arg1;
    if (M2C_FIELD(arg0, u16 *, 0x36) & 0x10) {
        var_s1 = M2C_FIELD(arg0, u8 *, 0x3C);
    }
    func_80078598(arg0);
    switch (var_s1) {
    case 1:
        func_801C51F8(arg0, 2, 1);
        break;
    case 2:
        func_801C51F8(arg0, 5, 4);
        break;
    case 0:
        if (func_801C50F4(arg0) != 0) {
            func_80076578(arg0, 0, 1, &func_801C4D98);
            M2C_FIELD(arg0, s16 *, 0x72) = 0x100;
        }
        break;
    case 3:
        func_8007790C(arg0, M2C_FIELD(arg0, u8 *, 0x64), 1, 0x400, arg0 + 0xCC, -1, &D_800764C4);
        break;
    case 4:
        if ((M2C_FIELD(arg0, u8 *, 0x64) == 8) && (func_8004EBF8(arg0 + 0x64) != 0)) {
            func_801C52E8(arg0, 1);
            func_80076578(arg0, 0, 1, &func_801C4D98);
        } else {
            func_800765E8(arg0, 8, 1, &func_801C4E00);
        }
        break;
    }
    if (!(M2C_FIELD(arg0, u16 *, 0x36) & 0x10)) {
        M2C_FIELD(arg0, u8 *, 0x3C) = var_s1;
    }
}
