/* ovl201 0x80088730  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005E92C();                            /* extern */
M2C_UNK func_8009C964(M2C_UNK);                     /* extern */
extern M2C_UNK func_8008AF0C;

void func_80088730(void *arg0) {
    if (obj_find_by_handle(&func_8008AF0C) == &g_ui_none) {
        func_8009C964(0);
        func_8005E92C();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
