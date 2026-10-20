/* ovl201 0x80089250  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8008B574();                              /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800897D0;

void func_80089250(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(func_8008B574(), s8 *, 2) = 0;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800897D0;
    }
}
