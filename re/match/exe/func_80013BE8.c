/* exe 0x80013BE8  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK MulMatrix(M2C_UNK, M2C_UNK *);              /* extern */
M2C_UNK MulMatrix2(M2C_UNK *, M2C_UNK);             /* extern */
M2C_UNK func_80013A88(s16, M2C_UNK);                /* extern */
M2C_UNK func_80013AFC(s16, M2C_UNK *);              /* extern */
M2C_UNK func_80013B74(s16, M2C_UNK *);              /* extern */

void func_80013BE8(void *arg0, M2C_UNK arg1) {
    union { s8 b[0x20]; s32 w; } sp_frame;

    func_80013AFC(M2C_FIELD(arg0, s16 *, 2), &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    func_80013A88(M2C_FIELD(arg0, s16 *, 0), arg1);
    MulMatrix(arg1, &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    func_80013B74(M2C_FIELD(arg0, s16 *, 4), &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    MulMatrix2(&(*(M2C_UNK *)(sp_frame.b + 0x0)), arg1);
}
