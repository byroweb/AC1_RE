/* exe 0x80013B74  size 116  variant sweep (tools/match/variants.py): old lever: swap M2C_FIELD(arg1, s16 *, 8 / M2C_FIELD(arg1, s16 *, 2, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 rcos(s16);                                      /* extern */
s16 rsin(s16);                                      /* extern */

void func_80013B74(s16 arg0, void *arg1) {
    s16 temp_s0;
    s16 temp_v0;

    temp_s0 = rsin(arg0);
    temp_v0 = rcos(arg0);
    M2C_FIELD(arg1, s16 *, 0) = temp_v0;
    M2C_FIELD(arg1, s16 *, 2) = (s16) -temp_s0;
    M2C_FIELD(arg1, s16 *, 8) = temp_v0;
    M2C_FIELD(arg1, s16 *, 4) = 0;
    M2C_FIELD(arg1, s16 *, 6) = temp_s0;
    M2C_FIELD(arg1, s16 *, 0xA) = 0;
    M2C_FIELD(arg1, s16 *, 0xC) = 0;
    M2C_FIELD(arg1, s16 *, 0xE) = 0;
    M2C_FIELD(arg1, s16 *, 0x10) = 0x1000;
}
