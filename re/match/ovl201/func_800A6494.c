/* ovl201 0x800A6494  size 140  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D71C();                            /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_80072838();                            /* extern */
M2C_UNK func_800804F0(void *, M2C_UNK *);           /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A6D24;

void func_800A6494(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
        func_8005D71C();
        func_8005DD38(0x3C0, 0x10, 0xF);
        func_8005DE08(0x3C0, 0x28, 0xF);
        func_80072838();
        func_800804F0(arg0, &func_800A6D24);
    }
}
