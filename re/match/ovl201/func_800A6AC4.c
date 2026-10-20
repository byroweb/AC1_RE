/* ovl201 0x800A6AC4  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800A5368();                            /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A6B24;

void func_800A6AC4(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        D_801A2574 = 2;
        func_800A5368();
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6B24;
    }
}
