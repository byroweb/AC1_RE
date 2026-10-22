/* fdat082 0x801C6568  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */

void func_801C6568(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 1;
    func_8004E98C(arg0 + 0x64, 0, 0, 0);
}
