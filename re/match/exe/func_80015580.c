/* exe 0x80015580  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx-regvars draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 CdControl(M2C_UNK, u8 *, M2C_UNK);              /* extern */
extern M2C_UNK D_8004A260;
extern u8 *D_8004A264;

void func_80015580(u8 *arg0) {
    u8 *var_s0;

    var_s0 = arg0;
    M2C_FIELD(var_s0, u8 *, 0) = 0;
    M2C_FIELD(var_s0, s8 *, 1) = 0;
    var_s0 += 0x28;
    if (var_s0 == &D_8004A260) {
        var_s0 -= 0x280;
    }
    D_8004A264 = var_s0;
    if (*var_s0 != 0) {
        do {

        } while (CdControl(0x16, var_s0 + 2, 0) == 0);
    }
}
