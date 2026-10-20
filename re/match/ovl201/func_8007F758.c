/* ovl201 0x8007F758  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8007D840(void *);                      /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8007D934;

void func_8007F758(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8007D840(arg0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8007D934;
    }
}
