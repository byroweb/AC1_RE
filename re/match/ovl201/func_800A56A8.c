/* ovl201 0x800A56A8  size 196  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800A56A8(s8 *arg0, s32 arg1) {
    s16 temp_v0;
    s16 var_a3;
    s32 temp_lo;
    s32 var_a1;
    s32 var_t0;
    s32 var_v1;
    s8 *var_a2;

    var_a1 = arg1;
    var_a2 = arg0;
    var_t0 = 0;
    if ((u32) (var_a1 & 0xFFFF) >= 0x3E8U) {
        var_a1 = 0x3E7;
    }
    var_v1 = 0x64;
    var_a3 = 0;
    do {
        temp_lo = (s32) (var_a1 & 0xFFFF) / var_v1;
        if (temp_lo > 0) {
            *var_a2 = temp_lo + 0x30;
            var_a2 += 1;
            var_t0 = 1;
            var_a1 -= temp_lo * var_v1;
        } else {
            if ((var_t0 & 0xFF) || (var_a3 == 2)) {
                *var_a2 = 0x30;
            } else {
                *var_a2 = 0x20;
            }
            var_a2 += 1;
        }
        temp_v0 = var_a3 + 1;
        var_a3 = temp_v0;
        var_v1 /= 0xA;
    } while (temp_v0 < 3);
    *var_a2 = 0;
}
