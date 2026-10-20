/* ovl201 0x80052334  size 504  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_80051D38(s32);                         /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
extern M2C_UNK D_8004AF88;
extern M2C_UNK D_8004AF94;
extern M2C_UNK D_8004AFA0;
extern M2C_UNK D_8004AFAC;
extern M2C_UNK D_8004AFB8;
extern M2C_UNK D_800B6EF8;
extern s16 D_801A2318;
extern M2C_UNK func_8005252C;

void func_80052334(void *arg0) {
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_80051D38(arg0 + 0xA1);
        if (M2C_FIELD(arg0, s32 *, 0x80) == 0) {
            func_8005D754(3, &D_800B6EF8, 0, 0, 4);
            func_8005D754(3, &D_800B6EF8, 0, 0x18, 4);
            func_8005D754(3, &D_800B6EF8, 0, 0x30, 4);
            func_8005D754(3, &D_800B6EF8, 0, 0x48, 4);
            func_8005D754(3, &D_800B6EF8, 0, 0x60, 4);
            func_8005D80C(2, &D_8004AF88, 0x10, 8, 4, 4);
            func_8005D80C(2, &D_8004AF94, 0x10, 8, 0x1C, 4);
            func_8005D80C(2, &D_8004AFA0, 0x10, 8, 0x34, 4);
            func_8005D80C(2, &D_8004AFAC, 0x10, 8, 0x4C, 4);
            func_8005D80C(2, &D_8004AFB8, 0x10, 8, 0x64, 4);
        }
        func_8005BA2C(0xA0, 0x20, 0x90, 0xA0, 5, 1, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8005252C;
    }
}
