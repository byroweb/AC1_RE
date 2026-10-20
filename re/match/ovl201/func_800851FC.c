/* ovl201 0x800851FC  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8007092C();                            /* extern */
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800709D4;

void func_800851FC(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800709D4;
        func_8007092C();
        M2C_FIELD(arg0, s8 *, 0x88) = 1;
    }
}
