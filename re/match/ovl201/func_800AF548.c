/* ovl201 0x800AF548  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8005C2D0;
extern M2C_UNK func_8005CC48;

void func_800AF548(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        if (M2C_FIELD(arg0, u8 *, 0x8A) != 0) {
            obj_find_by_handle(&func_8005CC48)->kill = 1;
        }
        M2C_FIELD(M2C_FIELD(arg0, void **, 0x7C), s32 *, 0x74) = (s32) M2C_FIELD(arg0, s32 *, 0x80);
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
