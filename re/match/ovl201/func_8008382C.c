/* ovl201 0x8008382C  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80081290();                            /* extern */
M2C_UNK func_80083894(s32, s32, M2C_UNK);           /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800813A8;

void func_8008382C(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_80081290();
        func_80083894(arg0 + 0x9A, arg0 + 0x9B, 4);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800813A8;
    }
}
