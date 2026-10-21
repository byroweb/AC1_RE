/* fdat002 0x801C5E84  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_801C5D8C();                            /* extern */

void func_801C5E84(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 1;
    func_801C5D8C();
}
