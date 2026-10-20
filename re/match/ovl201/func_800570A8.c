/* ovl201 0x800570A8  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800AEB30(void *, M2C_UNK *);           /* extern */
extern M2C_UNK func_80057108;
extern M2C_UNK func_8005C2D0;

void func_800570A8(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_800AEB30(arg0, &func_80057108);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
