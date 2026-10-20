/* ovl201 0x8004E22C  size 144  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_800B6EA0;
extern s16 D_801A2318;
extern M2C_UNK func_8004E18C;
extern M2C_UNK func_8005BA2C;

void func_8004E22C(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005CAE0(&D_800B6EA0, 0x10, 8, 6);
        SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, s16 *, 0x7C) = 0;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004E18C;
    }
}
