/* ovl201 0x800A69AC  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A57F0;

void func_800A69AC(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        ui_pool_set_light(4, 0x15);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A57F0;
    }
}
