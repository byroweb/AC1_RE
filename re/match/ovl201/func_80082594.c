/* ovl201 0x80082594  size 244  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_80082688;
extern M2C_UNK func_800A2D0C;

void func_80082594(void *arg0) {
    void *temp_a0;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        temp_a0 = M2C_FIELD(arg0, void **, 0x7C);
        if (M2C_FIELD(temp_a0, s32 *, 0x70) == &func_800A2D0C) {
            M2C_FIELD(temp_a0, s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
            M2C_FIELD(arg0, s8 *, 1) = 1;
            return;
        }
        func_8005BA2C(0xB0, 0x18, 0x80, 0x90, 5, 1, 0);
        func_8005BA2C(0x20, 0x28, 0x80, 0xA8, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80082688;
    }
}
