/* ovl201 0x8008A418  size 164  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8008B610();                            /* extern */
extern M2C_UNK D_8004CBD4;
extern M2C_UNK D_8004CBE4;
extern M2C_UNK D_800B8A98;
extern s8 D_801F3751;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8008A4BC;

void func_8008A418(void *arg0) {
    M2C_UNK *var_a0;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8008B610();
        var_a0 = &D_8004CBE4;
        if (D_801F3751 >= 0) {
            var_a0 = &D_8004CBD4;
        }
        func_8005CB94(var_a0, 8, 4, 0xE);
        func_8005D754(2, &D_800B8A98, 0, 0, 0xE);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8008A4BC;
    }
}
