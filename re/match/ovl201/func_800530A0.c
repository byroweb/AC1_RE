/* ovl201 0x800530A0  size 264  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051B8C(void *);                      /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
void *func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
extern M2C_UNK D_8004B02C;
extern M2C_UNK D_8004B034;
extern M2C_UNK D_800B6F10;
extern M2C_UNK func_800531A8;
extern M2C_UNK func_8005BA2C;

void func_800530A0(void *arg0) {
    void *temp_v0;
    void *temp_v0_2;

    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        temp_v0 = func_8005D80C(2, &D_8004B02C, 0x10, 0x24, 4, 4);
        M2C_FIELD(arg0, void **, 0x88) = temp_v0;
        M2C_FIELD(temp_v0, s16 *, 0x16) = 0x10;
        temp_v0_2 = func_8005D80C(2, &D_8004B034, 0x10, 0x24, 0x1C, 4);
        M2C_FIELD(arg0, void **, 0x8C) = temp_v0_2;
        M2C_FIELD(temp_v0_2, s16 *, 0x16) = 0x10;
        func_8005D754(3, &D_800B6F10, 0, 0, 4);
        func_8005D754(3, &D_800B6F10, 0, 0x18, 4);
        func_80051B8C(arg0 + 0xA0);
        M2C_FIELD(arg0, s32 *, 0x98) = 2;
        M2C_FIELD(arg0, s32 *, 0x9C) = 2;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800531A8;
    }
}
