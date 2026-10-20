/* ovl201 0x800A2EB0  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
extern M2C_UNK func_800A2F14;

void func_800A2EB0(void *arg0) {
    u16 temp_v0;

    if ((func_800517C8() << 0x10) == 0) {
        temp_v0 = M2C_FIELD(arg0, u16 *, 0x84) + 1;
        M2C_FIELD(arg0, u16 *, 0x84) = temp_v0;
        if ((s16) temp_v0 >= 0xFA) {
            M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A2F14;
        }
    }
}
