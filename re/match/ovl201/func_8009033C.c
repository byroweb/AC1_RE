/* ovl201 0x8009033C  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern M2C_UNK func_800903A8;

void func_8009033C(void *arg0) {
    if (obj_find_by_id(7) == &g_ui_none) {
        func_8005BA2C(0xA8, 0x68, 0x90, 0x70, 7, 1, 0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800903A8;
    }
}
