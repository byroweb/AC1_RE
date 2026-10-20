/* ovl201 0x80068BC0  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_8006BA08();                            /* extern */
extern M2C_UNK func_80068C08;

void func_80068BC0(void *arg0) {
    func_8006BA08();
    if ((func_800517C8() << 0x10) == 0) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80068C08;
    }
}
