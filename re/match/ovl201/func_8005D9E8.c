/* ovl201 0x8005D9E8  size 200  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8005CB94(s32, M2C_UNK, M2C_UNK, s16);      /* extern */
s32 func_8005D754(M2C_UNK, M2C_UNK *, s16, M2C_UNK, s32); /* extern */
extern M2C_UNK D_800B70A0;
extern M2C_UNK D_800B70AC;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8005DAB0;

void func_8005D9E8(void *arg0) {
    if (obj_find_by_handle(&func_8005BA2C) == &g_ui_none) {
        M2C_FIELD(arg0, s32 *, 0x7C) = func_8005D754(1, &D_800B70AC, (s16) ((M2C_FIELD(arg0, s16 *, 0x88) * 0x45) + 4), 0x18, (s32) M2C_FIELD(arg0, s16 *, 0x94));
        M2C_FIELD(arg0, s32 *, 0x80) = func_8005CB94(M2C_FIELD(arg0, s32 *, 0x8C), 8, 4, M2C_FIELD(arg0, s16 *, 0x94));
        M2C_FIELD(arg0, s32 *, 0x84) = func_8005D754(2, &D_800B70A0, 0, 0, (s32) M2C_FIELD(arg0, s16 *, 0x94));
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8005DAB0;
    }
}
