/* ovl201 0x800A3F90  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005CC48();                            /* extern */
M2C_UNK func_800810F0(void *, M2C_UNK *, M2C_UNK);  /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A3FFC;

void func_800A3F90(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005CC48();
        func_800810F0(arg0, &func_800A3FFC, 0x50);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
