/* fdat006 0x801C6068  size 40  permuter tier (tools/loop/permute.py): levers index D_801D0B94 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern u8 D_801D0B94[];
void func_801C6068(s32 arg0) {
    s32 *temp_v0;

    temp_v0 = (*(s32 **)(D_801D0B94 + ((arg0 << 6))));
    if (temp_v0 != NULL) {
        *temp_v0 = 0;
    }
}
