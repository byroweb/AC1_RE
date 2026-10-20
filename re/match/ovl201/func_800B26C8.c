/* ovl201 0x800B26C8  size 216  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, s16, s32); /* extern */
s32 func_800B1F74(s32);                             /* extern */
M2C_UNK func_800B27A0(void *);                      /* extern */
extern M2C_UNK D_800BDEE4;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_800B2AC4;

void func_800B26C8(void *arg0) {
    s16 temp_v0;
    s16 var_s0;

    if ((obj_find_by_handle(&func_8005BA2C) == &g_ui_none) && ((func_800517C8() << 0x10) == 0)) {
        M2C_FIELD(arg0, s32 *, 0x94) = func_800B1F74(arg0 + 0x98);
        var_s0 = 0;
        do {
            func_8005D754(3, &D_800BDEE4, 0, (s16) (var_s0 * 0x18), 4);
            temp_v0 = var_s0 + 1;
            var_s0 = temp_v0;
        } while (temp_v0 < 6);
        func_800B27A0(arg0);
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B2AC4;
    }
}
