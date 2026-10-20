/* ovl202 0x80057134  size 40  permuter tier (tools/loop/permute.py): levers ptype var_v0 s8->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s8 D_801AC8A4;

void func_80057134(void) {
    s32 var_v1;
    u8 *var_v0;

    var_v1 = 8;
    var_v0 = &D_801AC8A4;
    do {
        *var_v0 = 0xFF;
        var_v1 -= 1;
        var_v0 += 0x18;
    } while (var_v1 != 0);
}
