/* ovl203 0x8007CB08  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void *D_80039D18;

void func_8007CB08(void) {
    void *temp_v1;

    temp_v1 = D_80039D18 + ((M2C_FIELD(D_80039D18, u8 *, 0x18) * 0x3C) + 0x2DA);
    M2C_FIELD(D_80039D18, s16 *, 0x218) = (s16) (((u32) (M2C_FIELD(temp_v1, u8 *, 7) * 5) >> 5) + 1);
    M2C_FIELD(D_80039D18, s16 *, 0x21A) = (s16) (((s32) (M2C_FIELD(temp_v1, u8 *, 8) * 0xF) >> 5) + 1);
    M2C_FIELD(D_80039D18, s16 *, 0x21C) = (s16) (((s32) (M2C_FIELD(temp_v1, u8 *, 9) * 0x4B) >> 5) + 1);
}
