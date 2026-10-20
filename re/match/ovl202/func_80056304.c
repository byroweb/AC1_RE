/* ovl202 0x80056304  size 216  zero-agent sweep (tools/loop/sweep.py): m2c ctx-regvars draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78(s32);                             /* extern */

void func_80056304(void *arg0, s32 arg1) {
    s32 var_s0;
    s32 var_s1;
    s32 var_s3;
    s32 var_s4;
    s32 var_v1;
    void *var_s2;

    var_s0 = M2C_FIELD(arg0, s32 *, 0x80);
    var_s2 = M2C_FIELD(arg0, void **, 0x88);
    var_v1 = var_s0 >> 1;
    if (var_s0 < arg1) {
        var_s3 = arg1 / var_s0;
        var_s0 -= 1;
        var_s1 = 0;
        if (var_s0 != -1) {
            var_s4 = -1;
            do {
                M2C_FIELD(var_s2, s32 *, 0xC) = (s32) (var_s1 + func_80014D78(var_s3));
                var_s1 += var_s3;
                var_s0 -= 1;
                var_s2 += 0x20;
            } while (var_s0 != -1);
        }
    } else {
        var_s0 -= 1;
        var_s1 = 0;
        if (var_s0 != -1) {
            do {
                var_v1 -= arg1;
                M2C_FIELD(var_s2, s32 *, 0xC) = var_s1;
                if (var_v1 <= 0) {
                    var_s1 += 1;
                    var_v1 += M2C_FIELD(arg0, s32 *, 0x80);
                }
                var_s0 -= 1;
                var_s2 += 0x20;
            } while (var_s0 != -1);
        }
    }
    M2C_FIELD(var_s2, s32 *, 0xC) = -1;
}
