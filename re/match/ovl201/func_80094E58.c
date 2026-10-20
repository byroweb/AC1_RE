/* ovl201 0x80094E58  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern M2C_UNK func_80094EC0;

void func_80094E58(void *arg0) {
    if (obj_find_by_id(0xF) == &g_ui_none) {
        func_8005BA2C(4, 0xC4, 0x90, 0x18, 0xF, 0, 0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80094EC0;
    }
}
