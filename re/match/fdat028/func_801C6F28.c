/* fdat028 0x801C6F28  size 188  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_801C6F28(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0xBE) = (u16) (M2C_FIELD(arg0, u16 *, 0xBE) + M2C_FIELD(arg0, u16 *, 0x1E));
    M2C_FIELD(arg0, u8 *, 0xB8) = (u8) (M2C_FIELD(arg0, u8 *, 0xB8) + M2C_FIELD(arg0, u8 *, 0x28));
    M2C_FIELD(arg0, u8 *, 0xB9) = (u8) (M2C_FIELD(arg0, u8 *, 0xB9) + M2C_FIELD(arg0, u8 *, 0x29));
    M2C_FIELD(arg0, u8 *, 0xBA) = (u8) (M2C_FIELD(arg0, u8 *, 0xBA) + M2C_FIELD(arg0, u8 *, 0x2A));
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + ((s32) (M2C_FIELD(arg0, u16 *, 0x18) << 0x10) >> 0x14));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + ((s32) (M2C_FIELD(arg0, u16 *, 0x1A) << 0x10) >> 0x14));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + ((s32) (M2C_FIELD(arg0, u16 *, 0x1C) << 0x10) >> 0x14));
    M2C_FIELD(arg0, u16 *, 0x18) = (u16) (M2C_FIELD(arg0, u16 *, 0x18) + M2C_FIELD(arg0, u16 *, 0x20));
    M2C_FIELD(arg0, u16 *, 0x1A) = (u16) (M2C_FIELD(arg0, u16 *, 0x1A) + M2C_FIELD(arg0, u16 *, 0x22));
    M2C_FIELD(arg0, u16 *, 0x1C) = (u16) (M2C_FIELD(arg0, u16 *, 0x1C) + M2C_FIELD(arg0, u16 *, 0x24));
}
