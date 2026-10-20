/* ovl202 0x80085510  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_80085510(void *arg0) {
    M2C_FIELD(arg0, u16 *, 0) = (u16) (M2C_FIELD(arg0, u16 *, 0) + M2C_FIELD(arg0, u16 *, 0x18));
    M2C_FIELD(arg0, u16 *, 2) = (u16) (M2C_FIELD(arg0, u16 *, 2) + M2C_FIELD(arg0, u16 *, 0x1A));
    M2C_FIELD(arg0, u16 *, 4) = (u16) (M2C_FIELD(arg0, u16 *, 4) + M2C_FIELD(arg0, u16 *, 0x1C));
    M2C_FIELD(arg0, u16 *, 0xC6) = (u16) (M2C_FIELD(arg0, u16 *, 0xC6) + M2C_FIELD(arg0, u16 *, 0x20));
    M2C_FIELD(arg0, u16 *, 0xC8) = (u16) (M2C_FIELD(arg0, u16 *, 0xC8) + M2C_FIELD(arg0, u16 *, 0x22));
    M2C_FIELD(arg0, u16 *, 0xCA) = (u16) (M2C_FIELD(arg0, u16 *, 0xCA) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 0x1A) = (u16) (M2C_FIELD(arg0, u16 *, 0x1A) + 6);
}
