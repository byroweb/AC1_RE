/* ovl201 0x8006F590  size 88  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK MoveImage(M2C_UNK *, M2C_UNK, M2C_UNK);     /* extern */
M2C_UNK StoreImage(M2C_UNK *, s32);                 /* extern */
extern M2C_UNK D_800B813C;

void func_8006F590(void *arg0) {
    MoveImage(&D_800B813C, 0x39A, 0);
    StoreImage(&D_800B813C, M2C_FIELD(arg0, s32 *, 0x88));
    DrawSync(0);
}
