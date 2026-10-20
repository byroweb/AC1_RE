/* ovl201 0x8007FF64  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8007CFC0;

void func_8007FF64(void *arg0) {
    s32 var_v0;

    if (obj_find_by_handle(&func_8007CFC0) != &g_ui_none) {
        var_v0 = (M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s8 *, 0x84) * 0x18) + 0x20;
    } else {
        var_v0 = M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s8 *, 0x84) * 0x18;
    }
    M2C_FIELD(arg0, s32 *, 0x4C) = var_v0;
}
