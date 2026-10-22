/* fdat038 0x801C4CA0  size 312  sweep.py best draft: ctx, closeness 0.974 aligned / 0.974 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_800132CC(s16, s16, s16);                   /* extern */
M2C_UNK func_80014338(M2C_UNK *, M2C_UNK, void *);  /* extern */
M2C_UNK func_800143B8(void *, M2C_UNK, M2C_UNK, void *); /* extern */
M2C_UNK func_80014728(s32, M2C_UNK *);              /* extern */
s32 func_8004F080(void *, void *);                  /* extern */
s16 func_80075EB4();                                /* extern */

void func_801C4CA0(void *arg0) {
    M2C_UNK sp10;
    M2C_UNK (*temp_v0_3)(void *, u8);
    s16 temp_v0_2;
    s16 var_a1;
    s32 var_s1;
    u8 temp_v0;
    void *temp_a0;

    var_a1 = func_80075EB4();
    temp_v0 = M2C_FIELD(arg0, u8 *, 0x3C);
    var_s1 = 0;
    switch (temp_v0) {
    case 1:
block_2:
        temp_v0_2 = func_800132CC(M2C_FIELD(arg0, s16 *, 0x12), var_a1, M2C_FIELD(M2C_FIELD(arg0, void **, 4), s16 *, 8));
        M2C_FIELD(arg0, s16 *, 0x12) = temp_v0_2;
        func_80014728(temp_v0_2 + var_s1, &sp10);
        func_80014338(&sp10, 0x3E8, arg0 + 0xCC);
        break;
    case 2:
        var_a1 += 0x200;
        goto block_2;
    case 3:
        var_a1 -= 0x200;
        goto block_2;
    case 6:
        var_a1 += 0x800;
        var_s1 = 0x800;
        goto block_2;
    case 7:
        var_a1 += 0x600;
        var_s1 = 0x800;
        goto block_2;
    case 8:
        var_a1 += 0xA00;
        var_s1 = 0x800;
        goto block_2;
    case 4:
        var_a1 += 0x400;
        goto block_2;
    case 5:
        var_a1 -= 0x400;
        goto block_2;
    default:
        temp_a0 = arg0 + 0xCC;
        func_800143B8(temp_a0, 0x1F4, 0x3E8, temp_a0);
        break;
    }
    if (func_8004F080(arg0 + 0x64, arg0 + 8) == 0) {
        temp_v0_3 = M2C_FIELD(arg0, M2C_UNK (**)(void *, u8), 0x50);
        if (temp_v0_3 != NULL) {
            temp_v0_3(arg0, M2C_FIELD(arg0, u8 *, 0x41));
        }
    }
}
