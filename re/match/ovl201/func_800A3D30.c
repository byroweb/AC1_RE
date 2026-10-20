/* ovl201 0x800A3D30  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80050804(M2C_UNK, M2C_UNK);            /* extern */
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A304C;

void func_800A3D30(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_80050804(4, 0);
        M2C_FIELD(arg0, s8 *, 0x9F) = 1;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A304C;
    }
}
