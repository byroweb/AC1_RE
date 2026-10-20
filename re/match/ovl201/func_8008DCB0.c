/* ovl201 0x8008DCB0  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004EFE8();                            /* extern */
M2C_UNK func_8005B12C();                            /* extern */
M2C_UNK func_8008D8AC();                            /* extern */
void func_80095124(UiObj *);                        /* extern */
extern s32 D_801A2574;
extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;
extern M2C_UNK func_80095094;

void func_8008DCB0(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8008D8AC();
        obj_find_by_handle(&func_80095094)->update = func_80095124;
        obj_find_by_handle(&func_8005CC48)->kill = 1;
        D_801A2574 = 0;
        func_8005B12C();
        func_8004EFE8();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
