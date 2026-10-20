/* ovl201 0x800556A8  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_800A2D0C;

void func_800556A8(void *arg0) {
    void *temp_a0;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        temp_a0 = M2C_FIELD(arg0, void **, 0x7C);
        M2C_FIELD(arg0, s8 *, 1) = 1;
        if (M2C_FIELD(temp_a0, s32 *, 0x70) == &func_800A2D0C) {
            M2C_FIELD(temp_a0, s8 *, 0x9C) = 0x4E;
        }
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
    }
}
