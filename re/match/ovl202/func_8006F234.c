/* ovl202 0x8006F234  size 204  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8006F04C(s32, s32, s32);               /* extern */

void func_8006F234(void *arg0) {
    s32 temp_v0;

    func_8006F04C(arg0 + 0xE4, arg0 + 0x9E, arg0 + 0xD0);
    temp_v0 = ((s32) -(M2C_FIELD(arg0, s32 *, 0xD0) + M2C_FIELD(arg0, s32 *, 0xA4)) >> 0xA) - 0x10;
    M2C_FIELD(arg0, u16 *, 0x50) = (u16) ((u32) (M2C_FIELD(arg0, s16 *, 0x9E) * temp_v0) >> 0xE);
    M2C_FIELD(arg0, u16 *, 0x52) = (u16) ((u32) (M2C_FIELD(arg0, s16 *, 0xA0) * temp_v0) >> 0xE);
    M2C_FIELD(arg0, u16 *, 0x54) = (u16) ((u32) (M2C_FIELD(arg0, s16 *, 0xA2) * temp_v0) >> 0xE);
    if (M2C_FIELD(arg0, s32 *, 0x60) != 0) {
        M2C_FIELD(arg0, u16 *, 0x50) = (u16) (M2C_FIELD(arg0, u16 *, 0x50) + M2C_FIELD(arg0, u16 *, 0x58));
        M2C_FIELD(arg0, u16 *, 0x52) = (u16) (M2C_FIELD(arg0, u16 *, 0x52) + M2C_FIELD(arg0, u16 *, 0x5A));
        M2C_FIELD(arg0, u16 *, 0x54) = (u16) (M2C_FIELD(arg0, u16 *, 0x54) + M2C_FIELD(arg0, u16 *, 0x5C));
    }
}
