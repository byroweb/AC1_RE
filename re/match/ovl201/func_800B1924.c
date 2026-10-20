/* ovl201 0x800B1924  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800AE838();                            /* extern */
extern M2C_UNK func_8005C2D0;

void func_800B1924(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x80), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x84);
        func_800AE838();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
