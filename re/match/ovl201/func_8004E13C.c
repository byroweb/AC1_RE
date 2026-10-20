/* ovl201 0x8004E13C  size 80  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8004E2BC;
extern M2C_UNK func_8005C2D0;

void func_8004E13C(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004E2BC;
    }
}
