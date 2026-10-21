/* fdat006 0x801C50C0  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78(M2C_UNK);                         /* extern */
M2C_UNK func_800759A4(s32, M2C_UNK, s16 *);         /* extern */

void func_801C50C0(s32 arg0, M2C_UNK arg1) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    (*(s16 *)(sp_frame.b + 0x0)) = func_80014D78(0x1F4) - 0xFA;
    (*(s16 *)(sp_frame.b + 0x4)) = func_80014D78(0x1F4) - 0xFA;
    (*(s16 *)(sp_frame.b + 0x2)) = 0;
    func_800759A4(arg0, arg1, &(*(s16 *)(sp_frame.b + 0x0)));
}
