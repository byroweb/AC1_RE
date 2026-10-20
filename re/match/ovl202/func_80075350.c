/* ovl202 0x80075350  size 200  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80075280(M2C_UNK);                         /* extern */
s32 func_800752F4(u16, M2C_UNK);                    /* extern */

void func_80075350(void *arg0, M2C_UNK arg1, M2C_UNK arg2, M2C_UNK arg3) {
    s32 temp_v0;
    s32 var_v1;

    if (arg0 == g_ac_entities) {
        var_v1 = func_80075280(arg1) * 2;
    } else {
        var_v1 = func_800752F4(M2C_FIELD(M2C_FIELD(arg0, void **, 0), u16 *, 0x14), arg1);
    }
    temp_v0 = M2C_FIELD(arg0, u16 *, 0x160) - var_v1;
    if (temp_v0 < 0) {
        M2C_FIELD(arg0, u16 *, 0x160) = 0U;
    } else {
        M2C_FIELD(arg0, u16 *, 0x160) = (u16) temp_v0;
    }
    if (M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK, M2C_UNK, M2C_UNK, s32), 0x5C) != NULL) {
        M2C_FIELD(arg0, M2C_UNK (**)(void *, M2C_UNK, M2C_UNK, M2C_UNK, s32), 0x5C)(arg0, arg1, arg2, arg3, var_v1);
    }
}
