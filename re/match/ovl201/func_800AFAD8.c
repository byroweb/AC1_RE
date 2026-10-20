/* ovl201 0x800AFAD8  size 292  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800AEA64(s32, M2C_UNK);                /* extern */
extern M2C_UNK D_8004D8C8;
extern M2C_UNK D_8004D8D4;
extern M2C_UNK D_8004D8E0;
extern M2C_UNK D_800BDDDC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800AFBFC;

void func_800AFAD8(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDDDC, 0, 0, 8);
        func_8005D754(3, &D_800BDDDC, 0, 0x18, 8);
        func_8005D754(3, &D_800BDDDC, 0, 0x30, 8);
        func_8005D80C(2, &D_8004D8C8, 0x10, 8, 4, 8);
        func_8005D80C(2, &D_8004D8D4, 0x10, 8, 0x1C, 8);
        func_8005D80C(2, &D_8004D8E0, 0x10, 8, 0x34, 8);
        func_800AEA64(arg0 + 0x89, 8);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AFBFC;
    }
}
