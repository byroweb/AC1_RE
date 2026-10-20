/* ovl201 0x800A3D90  size 204  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800689E0();                            /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A3F90;

void func_800A3D90(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0x18, 0x28, 0x120, 0x88, 3, 1, 0);
        func_8005BA2C(0x18, 0xB2, 0x90, 0x30, 5, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        func_800689E0();
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A3F90;
    }
}
