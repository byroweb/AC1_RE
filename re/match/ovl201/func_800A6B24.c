/* ovl201 0x800A6B24  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800A546C(void *);                      /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A6520;

void func_800A6B24(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_800A546C(arg0);
        ui_pool_set_light(4, 0x10);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6520;
    }
}
