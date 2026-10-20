/* ovl201 0x8008B9FC  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_800B8AEC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8008BA68;

void func_8008B9FC(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(func_8005CAE0(&D_800B8AEC, 0, 0, 0xA), s16 *, 0xA) = 4;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8008BA68;
    }
}
