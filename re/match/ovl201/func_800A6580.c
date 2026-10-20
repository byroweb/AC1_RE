/* ovl201 0x800A6580  size 228  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800A8560(s32);                         /* extern */
extern M2C_UNK D_8004D344;
extern M2C_UNK D_8004D350;
extern M2C_UNK D_800BDCD8;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A6664;

void func_800A6580(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDCD8, 0, 0, 0xA);
        func_8005D754(3, &D_800BDCD8, 0, 0x18, 0xA);
        func_8005D80C(2, &D_8004D344, 0x10, 8, 4, 0xA);
        func_8005D80C(2, &D_8004D350, 0x10, 8, 0x1C, 0xA);
        func_800A8560(arg0 + 0x8B);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6664;
    }
}
