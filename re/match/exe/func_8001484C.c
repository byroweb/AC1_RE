/* exe 0x8001484C  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8001484C(s32 arg0, void *arg1) {
    M2C_FIELD(arg1, s16 *, 0) = (s16) ((s32) (M2C_FIELD(arg1, s16 *, 0) * arg0) >> 0xC);
    M2C_FIELD(arg1, s16 *, 2) = (s16) ((s32) (M2C_FIELD(arg1, s16 *, 2) * arg0) >> 0xC);
    M2C_FIELD(arg1, s16 *, 4) = (s16) ((s32) (M2C_FIELD(arg1, s16 *, 4) * arg0) >> 0xC);
}
