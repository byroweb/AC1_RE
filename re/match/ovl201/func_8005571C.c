/* ovl201 0x8005571C  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D8D4(M2C_UNK *, void *, M2C_UNK, M2C_UNK); /* extern */
s32 func_800685A8(u8);                              /* extern */
extern M2C_UNK D_8004B27C;
extern M2C_UNK func_800531B8;
extern M2C_UNK func_800557CC;

void func_8005571C(void *arg0) {
    M2C_UNK *var_v0;

    if (func_800685A8(M2C_FIELD(arg0, u8 *, 0xA4)) != 0) {
        var_v0 = &func_800531B8;
    } else {
        func_8005D8D4(&D_8004B27C, arg0 + 0xA3, 0xA, 1);
        var_v0 = &func_800557CC;
    }
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = var_v0;
}
