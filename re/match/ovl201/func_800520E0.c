/* ovl201 0x800520E0  size 84  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8005C2D0;

void func_800520E0(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(arg0, s8 *, 1) = 1;
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
    }
}
