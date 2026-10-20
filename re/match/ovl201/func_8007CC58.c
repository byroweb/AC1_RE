/* ovl201 0x8007CC58  size 140  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80070A44(UiObj *);                        /* extern */
void *func_80072788(u8);                            /* extern */
M2C_UNK func_8008651C();                            /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80070890;

void func_8007CC58(void *arg0) {
    UiObj *temp_s0;
    void *temp_v0;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        temp_s0 = obj_find_by_handle(&func_80070890);
        func_8008651C();
        temp_v0 = func_80072788(M2C_FIELD(arg0, u8 *, 0x88));
        temp_s0->w.list.src = temp_v0;
        M2C_FIELD(temp_v0, s8 *, 0x7C) = 3;
        temp_s0->update = func_80070A44;
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
