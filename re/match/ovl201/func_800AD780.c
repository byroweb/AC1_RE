/* ovl201 0x800AD780  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800A9614();                            /* extern */
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800AD7FC;

void func_800AD780(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_800A9614();
        func_8005BA2C(0x48, 0x50, 0x88, 0x78, 9, 0, 0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AD7FC;
    }
}
