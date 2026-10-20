/* ovl201 0x80053A20  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004B0B0;
extern M2C_UNK func_80053A84;
extern M2C_UNK func_8005BA2C;

void func_80053A20(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005CB94(&D_8004B0B0, 8, 8, 0xA);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80053A84;
    }
}
