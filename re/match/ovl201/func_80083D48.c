/* ovl201 0x80083D48  size 168  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_80083DF0;

void func_80083D48(void *arg0) {
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        VSync(4);
        func_8005BA2C(0x10, 0xCA, 0x120, 0x18, 9, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80083DF0;
    }
}
