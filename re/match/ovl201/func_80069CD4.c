/* ovl201 0x80069CD4  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8006BA08();                            /* extern */
M2C_UNK func_80070154();                            /* extern */
M2C_UNK func_800701D4(M2C_UNK, void *, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80069D40;

void func_80069CD4(void *arg0) {
    func_8006BA08();
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80069D40;
        func_80070154();
        func_800701D4(0xC, arg0 + 0x91, 8, 8);
    }
}
