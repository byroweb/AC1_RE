/* ovl201 0x800AF410  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800B04BC(void *, M2C_UNK *);           /* extern */
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800AF470;

void func_800AF410(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
        func_800B04BC(arg0, &func_800AF470);
    }
}
