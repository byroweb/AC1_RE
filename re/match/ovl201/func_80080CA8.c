/* ovl201 0x80080CA8  size 184  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80080D60;

void func_80080CA8(void *arg0) {
    UiObj *temp_v0;
    void *temp_v1;

    temp_v0 = obj_find_by_handle(func_8005C2D0);
    if (temp_v0 == &g_ui_none) {
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, 0x28, 0x88, 0xA8, 3);
        temp_v1 = M2C_FIELD(arg0, void **, 0x7C);
        if (temp_v1 == temp_v0) {
            M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80080D60;
            return;
        }
        M2C_FIELD(temp_v1, s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
