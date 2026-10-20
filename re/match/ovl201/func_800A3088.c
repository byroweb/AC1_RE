/* ovl201 0x800A3088  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
s32 func_800A4340(M2C_UNK);                         /* extern */
UiObj *func_800A4408();                             /* extern */
extern M2C_UNK func_800A310C;

void func_800A3088(void *arg0) {
    UiObj *var_v0;

    if ((func_800517C8() << 0x10) == 0) {
        M2C_FIELD(arg0, s32 *, 0x7C) = func_800A4340(0x4C);
        M2C_FIELD(arg0, s32 *, 0x80) = func_800A4340(0x52);
        M2C_FIELD(arg0, s16 *, 0x84) = 0;
        M2C_FIELD(arg0, s16 *, 0x86) = 0;
        if (M2C_FIELD(arg0, u8 *, 0x9F) != 0) {
            var_v0 = &g_ui_none;
        } else {
            var_v0 = func_800A4408();
        }
        M2C_FIELD(arg0, UiObj **, 0x8C) = var_v0;
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A310C;
    }
}
