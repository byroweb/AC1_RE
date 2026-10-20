/* ovl201 0x8005D15C  size 192  zero-agent sweep (tools/loop/sweep.py): m2c ctx-regvars draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK MoveImage(M2C_UNK *, s16, s16);             /* extern */
extern M2C_UNK D_800B7400;

void func_8005D15C(u8 *arg0, s16 arg1, s16 arg2) {
    M2C_UNK *var_s2;
    s16 var_s1;
    s16 var_s3;
    s32 var_v0_2;
    u8 *var_s0;
    u8 temp_v1;
    u8 var_v0;

    var_s0 = arg0;
    var_s1 = arg1;
    var_v0 = *var_s0;
    var_s3 = arg2;
    if (var_v0 != 0) {
        var_s2 = &D_800B7400;
        var_s2 = &D_800B7400;
        do {
            M2C_FIELD(&D_800B7400, s16 *, 0) = (s16) ((((var_v0 - 0x20) & 0x1F) * 2) + 0x1C0);
            temp_v1 = *var_s0;
            var_v0_2 = temp_v1 - 0x20;
            if (var_v0_2 < 0) {
                var_v0_2 = temp_v1 - 1;
            }
            M2C_FIELD(&D_800B7400, s16 *, 2) = (s16) ((var_v0_2 >> 5) * 0x10);
            MoveImage(&D_800B7400, var_s1, var_s3);
            var_s0 += 1;
            var_v0 = *var_s0;
            var_s1 += 2;
        } while (var_v0 != 0);
    }
    DrawSync(0);
}
