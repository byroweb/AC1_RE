/* ovl203 0x8007A590  size 92  permuter tier (tools/loop/permute.py): levers bytes var_v1 += 0x30, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern void *D_80039D18;

void func_8007A590(void) {
    s32 *var_v1;
    s32 var_a0;
    void *var_v0;

    var_a0 = 8;
    var_v1 = D_80039D18 + 0x84;
    var_v0 = D_80039D18 + 0x90;
    do {
        *var_v1 = 0;
        M2C_FIELD(var_v0, s16 *, 2) = 0;
        M2C_FIELD(var_v0, s16 *, 0) = 0;
        var_v0 += 0x30;
        var_a0 -= 1;
        var_v1 = (s32 *)((u8 *)var_v1 + 0x30);
    } while (var_a0 != 0);
    M2C_FIELD(D_80039D18, s32 *, 0x80) = 0;
    M2C_FIELD(D_80039D18, s32 *, 0x24) = 0;
    M2C_FIELD(D_80039D18, s16 *, 0x2E) = 0;
    M2C_FIELD(D_80039D18, s16 *, 0x2C) = 0;
    M2C_FIELD(D_80039D18, s16 *, 0x2A) = 0;
    M2C_FIELD(D_80039D18, u16 *, 0x28) = (u16) M2C_FIELD(D_80039D18, u16 *, 0x220);
}
