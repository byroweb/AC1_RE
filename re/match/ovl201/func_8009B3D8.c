/* ovl201 0x8009B3D8  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8009B3D8(s8 *arg0) {
    s16 temp_v0;
    s16 var_a1;
    s8 *var_a0;
    void *var_v1;

    var_a0 = arg0;
    var_a1 = 0;
    var_v1 = var_a0 + 0x6C;
    do {
        temp_v0 = var_a1 + 1;
        var_a1 = temp_v0;
        *var_a0 = 0;
        M2C_FIELD(var_v1, s8 *, -0x6B) = 0;
        M2C_FIELD(var_v1, s32 *, 0) = 0;
        var_v1 += 0xD0;
        var_a0 += 0xD0;
    } while (temp_v0 < 0x1FE);
}
