/* ovl201 0x800A407C  size 128  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK D_800BDC5C;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800A40FC;

void func_800A407C(void *arg0) {
    if ((obj_find_by_handle(&func_8005BA2C) == &g_ui_none) && ((func_800517C8() << 0x10) == 0)) {
        func_8005D754(1, &D_800BDC5C, 4, 0, 4);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A40FC;
    }
}
