/* ovl201 0x8007AC74  size 240  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
s32 func_800517C8();                                /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8007437C;
extern M2C_UNK func_8007AD64;

void func_8007AC74(void *arg0) {
    UiObj *temp_v0;

    if (((func_800517C8() << 0x10) == 0) && (obj_find_by_handle(&func_8005C2D0) == &g_ui_none)) {
        if (M2C_FIELD(arg0, u8 *, 0x84) != 0) {
            temp_v0 = obj_find_by_handle(&func_8007437C);
            temp_v0->kill = 1;
            temp_v0->update = ui_obj_nop2;
        }
        M2C_FIELD(arg0, u8 *, 0x84) = 1U;
        M2C_FIELD(arg0, s8 *, 0x85) = 0;
        func_8005BA2C(0x10, 0x28, 0x90, 0x80, 3, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8007AD64;
    }
}
