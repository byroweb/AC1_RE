/* ovl202 0x8004C91C  size 60  permuter tier (tools/loop/permute.py): levers index D_80041BD0 u8[], from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsVabClose(s16);                            /* extern */

extern u8 D_80041BD0[];
void func_8004C91C(s32 arg0) {
    s16 temp_a0;

    temp_a0 = (*(s16 *)(D_80041BD0 + ((arg0 * 8))));
    if (temp_a0 != -1) {
        SsVabClose(temp_a0);
    }
}
