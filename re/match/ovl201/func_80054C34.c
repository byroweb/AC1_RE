/* ovl201 0x80054C34  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_80054CD0;
extern M2C_UNK func_8005C2D0;

void func_80054C34(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0xC, 0x60, 0x128, 0x30, 9, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80054CD0;
    }
}
