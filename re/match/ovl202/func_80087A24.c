/* ovl202 0x80087A24  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80087A24(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0xBE) = (u16) (M2C_FIELD(arg0, u16 *, 0xBE) + M2C_FIELD(arg0, u16 *, 0x1E));
    M2C_FIELD(arg0, u8 *, 0xB8) = (u8) (M2C_FIELD(arg0, u8 *, 0xB8) - M2C_FIELD(arg0, u8 *, 0x28));
    M2C_FIELD(arg0, u8 *, 0xB9) = (u8) (M2C_FIELD(arg0, u8 *, 0xB9) - M2C_FIELD(arg0, u8 *, 0x29));
    M2C_FIELD(arg0, u8 *, 0xBA) = (u8) (M2C_FIELD(arg0, u8 *, 0xBA) - M2C_FIELD(arg0, u8 *, 0x2A));
}
