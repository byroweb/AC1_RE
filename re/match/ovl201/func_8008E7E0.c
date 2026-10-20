/* ovl201 0x8008E7E0  size 160  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32, s32); /* extern */
extern s8 D_8019C094;
extern s16 D_801A2318;
extern M2C_UNK func_8008F3B4;

void func_8008E7E0(void *arg0) {
    if (obj_find_by_id(9) == &g_ui_none) {
        func_8005BA2C(0x8C, 0x18, 0xB0, D_8019C094 * 0x18, 9, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8008F3B4;
    }
}
