/* ovl201 0x8005DD38  size 208  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK MoveImage(M2C_UNK *, s16, s16);             /* extern */
extern M2C_UNK D_800B7420;
extern M2C_UNK D_800B7428;
extern M2C_UNK D_800B7430;

void func_8005DD38(s16 arg0, s16 arg1, s16 arg2) {
    s16 temp_v0;
    s16 var_s1;
    s16 var_s2;

    MoveImage(&D_800B7420, arg0, arg1);
    var_s1 = arg0 + 2;
    var_s2 = 0;
    if (arg2 > 0) {
        do {
            MoveImage(&D_800B7428, var_s1, arg1);
            temp_v0 = var_s2 + 1;
            var_s2 = temp_v0;
            var_s1 += 2;
        } while (temp_v0 < arg2);
    }
    MoveImage(&D_800B7430, var_s1, arg1);
}
