/* ovl201 0x80093FC8  size 164  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_80095F58();                            /* extern */
extern M2C_UNK D_8004CE18;
extern M2C_UNK D_8004CE24;
extern M2C_UNK D_800B8BCC;
extern s8 D_8019C091;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800940A4;

void func_80093FC8(void *arg0) {
    M2C_UNK *var_a0;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_80095F58();
        var_a0 = &D_8004CE24;
        if (D_8019C091 == 0) {
            var_a0 = &D_8004CE18;
        }
        func_8005CB94(var_a0, 0x10, 4, 0xE);
        func_8005D754(2, &D_800B8BCC, 0, 0, 0xE);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800940A4;
    }
}
