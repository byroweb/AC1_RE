/* ovl201 0x800A5118  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_800517C8();                                /* extern */
M2C_UNK func_8005EB80(M2C_UNK);                     /* extern */
M2C_UNK func_80073740();                            /* extern */
M2C_UNK func_80074A18();                            /* extern */
extern M2C_UNK func_800A5170;

void func_800A5118(void *arg0) {
    if ((func_800517C8() << 0x10) == 0) {
        func_8005EB80(1);
        func_80074A18();
        func_80073740();
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A5170;
    }
}
