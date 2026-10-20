/* ovl201 0x800B2EC4  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EFE8();                            /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;

void func_800B2EC4(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        D_801A2574 = 0;
        M2C_FIELD(arg0, s8 *, 1) = 1;
        func_8004EFE8();
    }
}
