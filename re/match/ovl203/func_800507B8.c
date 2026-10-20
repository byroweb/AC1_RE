/* ovl203 0x800507B8  size 160  zero-agent sweep (tools/loop/sweep.py): m2c raw draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800507B8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, void *arg4) {
    void *temp_t1;

    temp_t1 = *(void **)0x80039D18;
    M2C_FIELD(arg4, s16 *, 0) = (s16) ((s32) (((s32) ((-arg1 * arg2) - (arg0 * arg3)) >> 0xC) * 0x17) / (s32) M2C_FIELD(temp_t1, u16 *, 0x270));
    M2C_FIELD(arg4, s16 *, 2) = (s16) ((s32) (((s32) ((arg1 * arg3) - (arg0 * arg2)) >> 0xC) * 0x17) / (s32) M2C_FIELD(temp_t1, u16 *, 0x270));
}
