/* ovl201 0x800867C4  size 116  permuter tier (tools/loop/permute.py): levers index D_801F3788 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_801F3788[];
s32 func_800867C4(void) {
    s32 var_a0;
    s32 var_v1;

    var_a0 = 1;
    var_v1 = 0x3F;
    do {
        if ((var_v1 != 0) && (var_v1 != 0x10) && (var_v1 != 0x1D) && (var_v1 != 0x27) && (var_v1 < 0x33) && !((*(u8 *)(D_801F3788 + (var_v1))) & 2)) {
            var_a0 = 0;
        }
        var_v1 -= 1;
    } while (var_v1 != -1);
    return var_a0;
}
