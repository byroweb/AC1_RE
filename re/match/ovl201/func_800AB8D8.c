/* ovl201 0x800AB8D8  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, s32);                /* extern */
M2C_UNK func_80051628();                            /* extern */
s32 func_800517C8();                                /* extern */
extern u8 D_80039CAE;

void func_800AB8D8(void) {
    if ((func_800517C8() << 0x10) != 0) {
        func_80051628();
        do {

        } while ((func_800517C8() << 0x10) != 0);
    }
    func_80051450(4, (D_80039CAE - 0x17) & 0xFFFF);
}
