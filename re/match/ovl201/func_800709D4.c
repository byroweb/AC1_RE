/* ovl201 0x800709D4  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80072788(u8);                              /* extern */
s32 func_80072838();                                /* extern */
M2C_UNK func_8008651C();                            /* extern */
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_80070A44;

void func_800709D4(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8008651C();
        M2C_FIELD(arg0, s32 *, 0x7C) = func_80072788(M2C_FIELD(arg0, u8 *, 0x88));
        M2C_FIELD(arg0, s32 *, 0x84) = func_80072838();
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80070A44;
    }
}
