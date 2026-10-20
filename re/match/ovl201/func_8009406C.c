/* ovl201 0x8009406C  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8008E880(s32, M2C_UNK *);              /* extern */
extern s32 D_80039CA4;

void func_8009406C(void) {
    union { s8 b[0x10]; s32 w; } sp_frame;

    func_8008E880(D_80039CA4, &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    func_8005D15C(&(*(M2C_UNK *)(sp_frame.b + 0x0)), 0x3F0, 0xE0);
}
