/* ovl201 0x8006F220  size 172  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void *func_80050FC4(M2C_UNK);                       /* extern */
M2C_UNK func_80051078(void *);                      /* extern */
s32 func_8006C4C0(s32, s16, s16);                   /* extern */
s32 func_8006F2CC(void *, s32, s32);                /* extern */

void func_8006F220(void *arg0) {
    s16 temp_a1;
    s16 temp_a2;
    s32 temp_v0;
    s32 var_v0;
    void *temp_v0_2;

    temp_a1 = M2C_FIELD(arg0, s16 *, 0x92);
    temp_a2 = M2C_FIELD(arg0, s16 *, 0x94);
    temp_v0 = func_8006C4C0(arg0 + 0x7C, temp_a1, temp_a2);
    if ((temp_v0 & 0xFF) != M2C_FIELD(arg0, u8 *, 0x86)) {
        temp_v0_2 = func_80050FC4(0x2000);
        var_v0 = temp_v0_2 + 2;
        M2C_FIELD(temp_v0_2, s8 *, 0) = (s8) temp_a1;
        M2C_FIELD(temp_v0_2, s8 *, 1) = (s8) temp_a2;
        if (var_v0 != temp_v0_2) {
            do {
                var_v0 = func_8006F2CC(arg0, var_v0, temp_v0 & 0xFF);
            } while (var_v0 != temp_v0_2);
        }
        func_80051078(temp_v0_2);
    }
}
