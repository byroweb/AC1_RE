/* fdat016 0x801C5DA0  size 116  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D30(M2C_UNK);                         /* extern */
s16 func_80014DAC(M2C_UNK);                         /* extern */
M2C_UNK func_80075A18(s32, M2C_UNK, s16 *);         /* extern */

void func_801C5DA0(s32 arg0, M2C_UNK arg1, s32 arg2) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    (*(s16 *)(sp_frame.b + 0x0)) = func_80014DAC(0x7D0);
    (*(s16 *)(sp_frame.b + 0x4)) = func_80014DAC(0x7D0);
    (*(s16 *)(sp_frame.b + 0x2)) = arg2 - func_80014D30(0x3E8);
    func_80075A18(arg0, arg1, &(*(s16 *)(sp_frame.b + 0x0)));
}
