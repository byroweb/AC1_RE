/* ovl202 0x800851A8  size 148  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800851A8(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + M2C_FIELD(arg0, u16 *, 0x1C));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + M2C_FIELD(arg0, u16 *, 0x1E));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + M2C_FIELD(arg0, u16 *, 0x20));
    M2C_FIELD(arg0, u16 *, 0xB8) = (u16) (M2C_FIELD(arg0, u16 *, 0xB8) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 0xBA) = (u16) (M2C_FIELD(arg0, u16 *, 0xBA) + M2C_FIELD(arg0, u16 *, 0x26));
    M2C_FIELD(arg0, u16 *, 0xBC) = (u16) (M2C_FIELD(arg0, u16 *, 0xBC) + M2C_FIELD(arg0, u16 *, 0x28));
    M2C_FIELD(arg0, u8 *, 0xC0) = (u8) (M2C_FIELD(arg0, u8 *, 0xC0) + M2C_FIELD(arg0, u8 *, 0x18));
    M2C_FIELD(arg0, u8 *, 0xC1) = (u8) (M2C_FIELD(arg0, u8 *, 0xC1) + M2C_FIELD(arg0, u8 *, 0x19));
    M2C_FIELD(arg0, u8 *, 0xC2) = (u8) (M2C_FIELD(arg0, u8 *, 0xC2) + M2C_FIELD(arg0, u8 *, 0x1A));
}
