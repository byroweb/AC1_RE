/* fdat012 0x801C5F54  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 func_80014D78(M2C_UNK);                         /* extern */
M2C_UNK func_80075A90(void *, M2C_UNK, s16 *);      /* extern */

void func_801C5F54(void *arg0, M2C_UNK arg1) {
    union { s8 b[0x8]; s32 w; } sp_frame;

    (*(s16 *)(sp_frame.b + 0x4)) = 0;
    (*(s16 *)(sp_frame.b + 0x0)) = 0;
    (*(s16 *)(sp_frame.b + 0x2)) = (M2C_FIELD(arg0, u16 *, 0x2C) - func_80014D78(0x12C)) - 0x1F4;
    func_80075A90(arg0, arg1, &(*(s16 *)(sp_frame.b + 0x0)));
}
