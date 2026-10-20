/* exe 0x80016D64  size 68  permuter tier (tools/loop/permute.py): levers swap var_v1 -= 1; / var_v0 += 0xA; + bytes var_v0 += 0xA, from the file draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtAllKeyOff(M2C_UNK);                     /* extern */
extern M2C_UNK D_80041DE0;

void func_80016D64(void) {
    M2C_UNK *var_v0;
    s32 var_v1;

    SsUtAllKeyOff(0);
    var_v0 = &D_80041DE0;
    var_v1 = 0x16;
    do {
        M2C_FIELD(var_v0, s8 *, 2) = -1;
        M2C_FIELD(var_v0, s16 *, 0) = 0;
        var_v0 = (M2C_UNK *)((u8 *)var_v0 + 0xA);
        var_v1 -= 1;
    } while (var_v1 != -1);
}
