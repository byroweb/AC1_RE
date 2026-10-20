/* ovl201 0x80054188  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005DED8(void *, M2C_UNK *, M2C_UNK *, M2C_UNK); /* extern */
extern M2C_UNK D_8004B0FC;
extern M2C_UNK func_80054258;
extern M2C_UNK func_8005C2D0;

void func_80054188(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005DED8(arg0, &func_80054258, &D_8004B0FC, 0xC);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
