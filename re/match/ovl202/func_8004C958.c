/* ovl202 0x8004C958  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsEnd();                                    /* extern */
M2C_UNK SsSetMVol(M2C_UNK, M2C_UNK);                /* extern */
M2C_UNK func_80016DA8();                            /* extern */

void func_8004C958(void) {
    SsSetMVol(0, 0);
    func_80016DA8();
    SsEnd();
}
