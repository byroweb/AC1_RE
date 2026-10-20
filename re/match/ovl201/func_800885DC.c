/* ovl201 0x800885DC  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EFE8();                            /* extern */
void func_8008AF9C(UiObj *);                        /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;
extern M2C_UNK func_8008AF0C;

void func_800885DC(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        obj_find_by_handle(&func_8008AF0C)->update = func_8008AF9C;
        obj_find_by_handle(&func_8005CC48)->kill = 1;
        D_801A2574 = 0;
        func_8004EFE8();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
