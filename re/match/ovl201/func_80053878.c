/* ovl201 0x80053878  size 172  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A38CC;

void func_80053878(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0x5C, 0x60, 0x88, 0x30, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), M2C_UNK **, 0x74) = &func_800A38CC;
        D_801A2574 = 0;
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
