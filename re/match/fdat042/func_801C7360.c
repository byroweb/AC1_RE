/* fdat042 0x801C7360  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_801C7360(void *arg0) {
    M2C_FIELD(arg0, u16 *, 8) = (u16) (M2C_FIELD(arg0, u16 *, 8) + M2C_FIELD(arg0, u16 *, 0x22));
    M2C_FIELD(arg0, u16 *, 0xA) = (u16) (M2C_FIELD(arg0, u16 *, 0xA) + M2C_FIELD(arg0, u16 *, 0x24));
    M2C_FIELD(arg0, u16 *, 0xC) = (u16) (M2C_FIELD(arg0, u16 *, 0xC) + M2C_FIELD(arg0, u16 *, 0x26));
}
