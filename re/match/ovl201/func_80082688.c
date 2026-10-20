/* ovl201 0x80082688  size 140  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80072788(u8);                            /* extern */
s32 func_80072838();                                /* extern */
M2C_UNK func_8008651C();                            /* extern */
extern M2C_UNK func_8005BA2C;

void func_80082688(void *arg0) {
    void *temp_s1;
    void *temp_v0;

    temp_s1 = M2C_FIELD(arg0, void **, 0x7C);
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8008651C();
        temp_v0 = func_80072788(M2C_FIELD(arg0, u8 *, 0x88));
        M2C_FIELD(temp_s1, void **, 0x7C) = temp_v0;
        M2C_FIELD(temp_v0, s8 *, 0x7C) = 5;
        M2C_FIELD(temp_s1, s32 *, 0x84) = func_80072838();
        M2C_FIELD(temp_s1, s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
