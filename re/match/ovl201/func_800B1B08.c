/* ovl201 0x800B1B08  size 272  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_8005CB94(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_800AEA64(s32, M2C_UNK);                /* extern */
extern M2C_UNK D_8004D9A8;
extern M2C_UNK D_8004D9B8;
extern M2C_UNK D_8004D9C8;
extern M2C_UNK D_800BDDDC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800B1C18;

void func_800B1B08(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        func_8005D754(3, &D_800BDDDC, 0, 0, 8);
        func_8005D754(3, &D_800BDDDC, 0, 0x18, 8);
        func_8005D754(3, &D_800BDDDC, 0, 0x30, 8);
        M2C_FIELD(func_8005CB94(&D_8004D9A8, 8, 4, 8), s16 *, 0x14) = 2;
        M2C_FIELD(func_8005CB94(&D_8004D9B8, 8, 0x1C, 8), s16 *, 0x14) = 2;
        M2C_FIELD(func_8005CB94(&D_8004D9C8, 8, 0x34, 8), s16 *, 0x14) = 2;
        func_800AEA64(arg0 + 0x89, 8);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B1C18;
    }
}
