/* ovl201 0x800AF658  size 232  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800AEA64(s32, M2C_UNK);                /* extern */
extern M2C_UNK D_8004D8B8;
extern M2C_UNK D_8004D8C0;
extern M2C_UNK D_800BDDDC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800AF740;

void func_800AF658(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDDDC, 0, 0, 8);
        func_8005D754(3, &D_800BDDDC, 0, 0x18, 8);
        func_8005D80C(2, &D_8004D8B8, 0x10, 0x20, 4, 8);
        func_8005D80C(2, &D_8004D8C0, 0x10, 0x20, 0x1C, 8);
        func_800AEA64(arg0 + 0x89, 8);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AF740;
    }
}
