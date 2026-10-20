/* ovl202 0x8004EAD0  size 12  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_8004EAD0(void *arg0) {
    return M2C_FIELD(arg0, s16 *, 0xA) == 0;
}
