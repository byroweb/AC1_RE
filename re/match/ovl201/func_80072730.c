/* ovl201 0x80072730  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EFE8();                            /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;

void func_80072730(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, s8 *, 1) = 1;
        D_801A2574 = 0;
        func_8004EFE8();
    }
}
