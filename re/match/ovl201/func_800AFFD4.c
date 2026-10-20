/* ovl201 0x800AFFD4  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_800B0400(s32);                         /* extern */
extern M2C_UNK D_800BDE00;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800B0048;

void func_800AFFD4(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDE00, 0, 0, 8);
        func_800B0400(arg0 + 0x85);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B0048;
    }
}
