/* ovl202 0x8004DE88  size 36  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8004DE88(void *arg0, void *arg1) {
    M2C_FIELD(arg1, u16 *, 0) = (u16) M2C_FIELD(arg0, u16 *, 0x20);
    M2C_FIELD(arg1, u16 *, 2) = (u16) M2C_FIELD(arg0, u16 *, 0x28);
    M2C_FIELD(arg1, u16 *, 4) = (u16) M2C_FIELD(arg0, u16 *, 0x30);
}
