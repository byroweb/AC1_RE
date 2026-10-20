/* ovl201 0x8005DCDC  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern M2C_UNK func_8005C2D0;

void func_8005DCDC(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        *M2C_FIELD(arg0, u8 **, 0x90) = M2C_FIELD(arg0, u8 *, 0x96);
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
