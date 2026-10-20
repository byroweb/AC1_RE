/* exe 0x80014804  size 72  variant sweep (tools/match/variants.py): args lever: args func_80013BE8 +arg0 proto, from the ctx-spframe draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK ApplyMatrixSV(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_80013BE8(s32, M2C_UNK *);                   /* extern */

void func_80014804(s32 arg0, M2C_UNK arg1, M2C_UNK arg2) {
    union { s8 b[0x20]; s32 w; } sp_frame;

    func_80013BE8(arg0, &(*(M2C_UNK *)(sp_frame.b + 0x0)));
    ApplyMatrixSV(&(*(M2C_UNK *)(sp_frame.b + 0x0)), arg1, arg2);
}
