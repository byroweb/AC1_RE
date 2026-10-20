/* exe 0x80016DA8  size 108  permuter tier (tools/loop/permute.py): levers bytes var_s1 += 8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtAllKeyOff(M2C_UNK);                     /* extern */
M2C_UNK SsVabClose(s16);                            /* extern */
extern s16 D_80041BD0;

void func_80016DA8(void) {
    s16 *var_s1;
    s16 temp_a0;
    s32 var_s0;

    SsUtAllKeyOff(0);
    var_s1 = &D_80041BD0;
    var_s0 = 0x41;
    do {
        temp_a0 = *var_s1;
        if (temp_a0 != -1) {
            SsVabClose(temp_a0);
        }
        var_s0 -= 1;
        var_s1 = (s16 *)((u8 *)var_s1 + 8);
    } while (var_s0 != -1);
}
