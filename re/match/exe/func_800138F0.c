/* exe 0x800138F0  size 180  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 rcos(M2C_UNK);                                  /* extern */
s32 rsin(M2C_UNK);                                  /* extern */

void func_800138F0(void *arg0, void *arg1, M2C_UNK arg2) {
    s32 temp_s0;
    s32 temp_v0;

    temp_s0 = rsin(arg2);
    temp_v0 = rcos(arg2);
    M2C_FIELD(arg1, s16 *, 0) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 0) * temp_v0) - (M2C_FIELD(arg0, s16 *, 4) * temp_s0)) >> 0xC);
    M2C_FIELD(arg1, s16 *, 4) = (s16) ((s32) ((M2C_FIELD(arg0, s16 *, 0) * temp_s0) + (M2C_FIELD(arg0, s16 *, 4) * temp_v0)) >> 0xC);
    M2C_FIELD(arg1, u16 *, 2) = (u16) M2C_FIELD(arg0, u16 *, 2);
}
