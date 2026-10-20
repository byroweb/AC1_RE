/* ovl201 0x8005EAA0  size 64  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
extern M2C_UNK func_8005EAE0;

void func_8005EAA0(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8005EAE0;
    }
}
