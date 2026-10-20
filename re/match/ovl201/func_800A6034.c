/* ovl201 0x800A6034  size 80  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_800744A8(void *, M2C_UNK *, M2C_UNK);  /* extern */
extern M2C_UNK func_800A6DA4;

void func_800A6034(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_800744A8(arg0, &func_800A6DA4, 1);
        M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = ui_obj_nop2;
    }
}
