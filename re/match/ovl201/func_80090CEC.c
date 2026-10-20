/* ovl201 0x80090CEC  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005B648(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern M2C_UNK func_80093A20;

void func_80090CEC(void *arg0) {
    if (obj_find_by_id(0xB) == &g_ui_none) {
        M2C_FIELD(func_8005B648(0x30, 0x1C, 0xE0, 0x90, 0xB, 1, 0), s16 *, 0x8A) = 1;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80093A20;
    }
}
