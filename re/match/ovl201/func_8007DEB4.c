/* ovl201 0x8007DEB4  size 120  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8007C6FC(UiObj *);                     /* extern */
void func_8007C7D8(UiObj *);                        /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8007C68C;

void func_8007DEB4(void *arg0) {
    UiObj *temp_v0;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        temp_v0 = obj_find_by_handle(&func_8007C68C);
        func_8007C6FC(temp_v0);
        temp_v0->update = func_8007C7D8;
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
