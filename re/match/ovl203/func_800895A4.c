/* ovl203 0x800895A4  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK MulRotMatrix(M2C_UNK *);                    /* extern */

void func_800895A4(void) {
    union { s8 b[0x20]; s32 w; } sp_frame;

    MulRotMatrix(&(*(M2C_UNK *)(sp_frame.b + 0x0)));
}
