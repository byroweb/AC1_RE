/* ovl201 0x80055BA4  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051DF0(void *, M2C_UNK *, M2C_UNK *, M2C_UNK, s32, s32, s32, s32); /* extern */
extern M2C_UNK D_8004B2E8;
extern M2C_UNK func_800531B8;
extern M2C_UNK func_8005C2D0;

void func_80055BA4(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_80051DF0(arg0, &func_800531B8, &D_8004B2E8, 0xC, 0x14, 0x68, 0x118, 0x20);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
