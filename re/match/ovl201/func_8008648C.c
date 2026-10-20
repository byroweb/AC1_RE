/* ovl201 0x8008648C  size 144  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80072788(u8);                            /* extern */
M2C_UNK func_8008651C();                            /* extern */
extern M2C_UNK func_8005BA2C;

void func_8008648C(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8008651C();
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), void **, 0x7C) = func_80072788(M2C_FIELD(arg0, u8 *, 0x88));
        M2C_FIELD(M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), void **, 0x7C), s8 *, 0x7C) = 2;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
