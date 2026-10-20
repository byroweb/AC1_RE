/* ovl201 0x8004E044  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004FB28(s8);                          /* extern */
extern M2C_UNK func_8004DCC0;
extern M2C_UNK func_8004E0B4;

void func_8004E044(void *arg0) {
    if (M2C_FIELD(M2C_FIELD(arg0, void **, 0x80), u8 *, 0x7E) != 0) {
        func_8004FB28(obj_find_by_handle(&func_8004DCC0)->w.raw[0x32]);
        M2C_FIELD(arg0, UiObj **, 0x98) = &g_ui_none;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004E0B4;
    }
}
