/* ovl201 0x8005EAE0  size 24  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s16 D_801A2560;

void func_8005EAE0(void *arg0) {
    D_801A2560 = 1;
    M2C_FIELD(arg0, s8 *, 1) = 1;
}
