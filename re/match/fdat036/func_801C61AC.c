/* fdat036 0x801C61AC  size 140  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800775B8(void *, M2C_UNK, M2C_UNK, M2C_UNK, void *, M2C_UNK *); /* extern */
extern s32 D_801A5DC8;
extern M2C_UNK func_801C5F98;

void func_801C61AC(void *arg0, u8 arg1) {
    u8 var_s1;

    var_s1 = arg1;
    if (M2C_FIELD(arg0, u16 *, 0x36) & 0x10) {
        var_s1 = M2C_FIELD(arg0, u8 *, 0x3C);
    }
    D_801A5DC8 = 0;
    func_800775B8(arg0, 0, 1, 0, arg0 + 0xCC, &func_801C5F98);
    if (!(M2C_FIELD(arg0, u16 *, 0x36) & 0x10)) {
        M2C_FIELD(arg0, u8 *, 0x3C) = var_s1;
    }
}
