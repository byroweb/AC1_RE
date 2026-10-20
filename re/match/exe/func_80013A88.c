/* exe 0x80013A88  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 rcos(s16);                                      /* extern */
s16 rsin(s16);                                      /* extern */

void func_80013A88(s16 arg0, void *arg1) {
    s16 temp_s0;
    s16 temp_v0;

    temp_s0 = rsin(arg0);
    temp_v0 = rcos(arg0);
    M2C_FIELD(arg1, s16 *, 0) = 0x1000;
    M2C_FIELD(arg1, s16 *, 2) = 0;
    M2C_FIELD(arg1, s16 *, 4) = 0;
    M2C_FIELD(arg1, s16 *, 6) = 0;
    M2C_FIELD(arg1, s16 *, 8) = temp_v0;
    M2C_FIELD(arg1, s16 *, 0xA) = (s16) -temp_s0;
    M2C_FIELD(arg1, s16 *, 0xC) = 0;
    M2C_FIELD(arg1, s16 *, 0xE) = temp_s0;
    M2C_FIELD(arg1, s16 *, 0x10) = temp_v0;
}
