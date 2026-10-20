/* ovl201 0x80083C00  size 88  permuter tier (tools/loop/permute.py): levers index D_80042AA8 u8[] + index D_800B8A24 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 func_8008CFD4(s8, s8);                          /* extern */

extern u8 D_80042AA8[];
extern u8 D_800B8A24[];
u8 func_80083C00(s32 arg0, s8 arg1) {
    s32 temp_a0;
    s8 var_v1;

    temp_a0 = arg0 & 0xFFFF;
    var_v1 = arg1;
    if (temp_a0 == 3) {
        var_v1 = arg1 + 0x10;
    }
    return (*(u8 *)(D_80042AA8 + (func_8008CFD4((*(s8 *)(D_800B8A24 + (temp_a0))), var_v1))));
}
