/* fdat088 0x801C68EC  size 48  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_801CC9D0;

void func_801C68EC(void *arg0) {
    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        D_801CC9D0 += 0x7D0;
    }
}
