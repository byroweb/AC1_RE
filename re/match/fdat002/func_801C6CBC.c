/* fdat002 0x801C6CBC  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_801C6DF0;
extern s32 D_801C6DF4;

void func_801C6CBC(s32 *arg0, s32 *arg1) {
    *arg0 += D_801C6DF4;
    *arg1 += D_801C6DF0;
}
