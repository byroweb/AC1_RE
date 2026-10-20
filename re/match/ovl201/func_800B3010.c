/* ovl201 0x800B3010  size 344  permuter tier (tools/loop/permute.py): levers index D_8017F590 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK VSync(M2C_UNK);                             /* extern */
M2C_UNK func_800518D4(M2C_UNK *, s32);              /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800B29AC(M2C_UNK *, s32);              /* extern */
extern M2C_UNK D_8004DB68;
extern M2C_UNK D_8004DB78;
extern M2C_UNK D_8017F578;
extern M2C_UNK D_8017F57C;
extern M2C_UNK func_800B3168;

extern u8 D_8017F590[];
void func_800B3010(void *arg0) {
    s32 temp_s2;

    temp_s2 = M2C_FIELD(arg0, u8 *, 0x99) + M2C_FIELD(arg0, u8 *, 0x98);
    VSync(0);
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_8005D80C(1, &D_8004DB68, 6, 8, 0x10, 4);
        func_8005D80C(1, &D_8004DB78, 6, 0x50, 0x10, 4);
        func_800518D4(&D_8017F578, temp_s2 + 1);
        func_800B29AC(&D_8017F57C, (*(s32 *)(D_8017F590 + ((temp_s2 * 0x10)))));
        func_8005D80C(1, &D_8017F578, 0x10, 0x28, 8, 4);
        func_8005D80C(1, &D_8017F57C, 0x10, 0x78, 8, 4);
        func_8005BA2C(0x10, 0x60, 0xB0, 0x30, 5, 1, 0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B3168;
    }
}
