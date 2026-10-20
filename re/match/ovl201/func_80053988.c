/* ovl201 0x80053988  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
s32 func_800684A0(u8);                              /* extern */
extern M2C_UNK func_80053A20;
extern M2C_UNK func_80054258;
extern M2C_UNK func_8005C2D0;

void func_80053988(void *arg0) {
    M2C_UNK *var_v0;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        if (func_800684A0(M2C_FIELD(arg0, u8 *, 0xA4)) != 0) {
            var_v0 = &func_80054258;
        } else {
            func_8005BA2C(0xC, 0x68, 0x128, 0x20, 9, 1, 0);
            var_v0 = &func_80053A20;
        }
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = var_v0;
    }
}
