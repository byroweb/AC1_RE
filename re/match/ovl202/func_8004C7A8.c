/* ovl202 0x8004C7A8  size 172  permuter tier (tools/loop/permute.py): levers etype D_80041BD0 M2C_UNK->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s16 SsVabOpenHead(s32, M2C_UNK);                    /* extern */
s16 SsVabTransBodyPartly(s32, M2C_UNK, s16);        /* extern */
M2C_UNK SsVabTransCompleted(M2C_UNK);               /* extern */
extern u8 D_80041BD0;

void func_8004C7A8(s32 arg0, s32 arg1, s32 arg2) {
    s16 temp_v0;
    s32 var_s1;
    void *temp_s0;

    var_s1 = arg2;
    temp_s0 = (arg0 * 8) + &D_80041BD0;
    temp_v0 = SsVabOpenHead(arg1, -1);
    M2C_FIELD(temp_s0, s16 *, 0) = temp_v0;
    if (temp_v0 != -1) {
        M2C_FIELD(temp_s0, s32 *, 4) = arg1;
loop_2:
        if (SsVabTransBodyPartly(var_s1, 0x800, M2C_FIELD(temp_s0, s16 *, 0)) != M2C_FIELD(temp_s0, s16 *, 0)) {
            SsVabTransCompleted(1);
            var_s1 += 0x800;
            goto loop_2;
        }
        SsVabTransCompleted(1);
    }
}
