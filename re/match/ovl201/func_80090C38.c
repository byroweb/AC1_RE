/* ovl201 0x80090C38  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_801A2574;

void func_80090C38(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0x42) = (u16) (M2C_FIELD(arg0, u16 *, 0x42) - (0x20 << D_801A2574));
}
