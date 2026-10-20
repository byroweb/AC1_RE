/* ovl201 0x800A636C  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_8008405C(void *, M2C_UNK *);           /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A6DA4;

void func_800A636C(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        VSync(4);
        func_8008405C(arg0, &func_800A6DA4);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
