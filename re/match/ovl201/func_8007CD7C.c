/* ovl201 0x8007CD7C  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800804F0(UiObj *, M2C_UNK);            /* extern */
extern M2C_UNK func_8005BA2C;

void func_8007CD7C(void *arg0) {
    UiObj *temp_v0;

    temp_v0 = obj_find_by_handle(&func_8005BA2C);
    if (temp_v0 == &g_ui_none) {
        func_800804F0(temp_v0, 0);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
