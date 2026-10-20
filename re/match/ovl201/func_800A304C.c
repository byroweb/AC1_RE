/* ovl201 0x800A304C  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, M2C_UNK);            /* extern */
extern M2C_UNK func_800A3088;

void func_800A304C(void *arg0) {
    func_80051450(4, 3);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A3088;
}
