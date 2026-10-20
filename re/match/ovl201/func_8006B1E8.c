/* ovl201 0x8006B1E8  size 104  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051078(s32);                         /* extern */
extern M2C_UNK func_8005C2D0;

void func_8006B1E8(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
        func_80051078(M2C_FIELD(arg0, s32 *, 0x88));
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
