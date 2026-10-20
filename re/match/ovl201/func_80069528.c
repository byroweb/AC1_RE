/* ovl201 0x80069528  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8006BA08();                            /* extern */
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_80068C08;

void func_80069528(void *arg0) {
    func_8006BA08();
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80068C08;
    }
}
