/* ovl201 0x80086FD4  size 40  permuter tier (tools/loop/permute.py): levers index D_801F37D4 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_801F37D4[];
void func_80086FD4(void) {
    s32 var_v0;

    var_v0 = 0x3F;
    do {
        (*(s8 *)(D_801F37D4 + (var_v0))) = -1;
        var_v0 -= 1;
    } while (var_v0 != -1);
}
