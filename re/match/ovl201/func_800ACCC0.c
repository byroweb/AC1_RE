/* ovl201 0x800ACCC0  size 252  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800ADB40(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004D51C;
extern M2C_UNK D_8004D528;
extern M2C_UNK D_800BDD34;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800ACDBC;

void func_800ACCC0(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDD34, 0, 0, 0xE);
        func_8005D754(3, &D_800BDD34, 0, 0x18, 0xE);
        func_8005D80C(2, &D_8004D51C, 0x10, 8, 4, 0xE);
        func_8005D80C(2, &D_8004D528, 0x10, 8, 0x1C, 0xE);
        func_800ADB40(arg0 + 0x83, 0, 0, 0xE);
        ui_pool_set_light(0xA, 0x10);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800ACDBC;
    }
}
