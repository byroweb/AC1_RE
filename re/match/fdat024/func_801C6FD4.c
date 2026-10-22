/* fdat024 0x801C6FD4  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_801C70F0;

void func_801C6FD4(void *arg0) {
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        D_801C70F0 += 0x1F4;
    }
}
