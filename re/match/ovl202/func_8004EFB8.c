/* ovl202 0x8004EFB8  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_8004EF74(void *, s16, u8);                 /* extern */

void func_8004EFB8(void *arg0, void *arg1) {
    M2C_FIELD(arg1, s16 *, 0) = func_8004EF74(arg0 + 0x20, M2C_FIELD(arg1, s16 *, 0), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg1, s16 *, 2) = func_8004EF74(arg0 + 0x28, M2C_FIELD(arg1, s16 *, 2), M2C_FIELD(arg0, u8 *, 3));
    M2C_FIELD(arg1, s16 *, 4) = func_8004EF74(arg0 + 0x30, M2C_FIELD(arg1, s16 *, 4), M2C_FIELD(arg0, u8 *, 3));
}
