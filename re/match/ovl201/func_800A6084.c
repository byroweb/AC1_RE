/* ovl201 0x800A6084  size 240  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32, s32); /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
s32 func_800842D4();                                /* extern */
extern s16 D_801A2318;
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A6174;

void func_800A6084(void *arg0) {
    s32 var_v1;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        D_801A2574 = 0;
        func_8005DD38(0x3C0, 0x10, 0x10);
        func_8005DE08(0x3C0, 0x28, 0xD);
        VSync(4);
        var_v1 = func_800842D4();
        if ((u32) (var_v1 & 0xFF) >= 7U) {
            var_v1 = 6;
        }
        func_8005BA2C(0x10, 0x40, 0x90, (var_v1 & 0xFF) * 0x18, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6174;
    }
}
