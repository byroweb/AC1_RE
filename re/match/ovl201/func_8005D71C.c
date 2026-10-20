/* ovl201 0x8005D71C  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK ClearImage(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK DrawSync(M2C_UNK);                          /* extern */
extern M2C_UNK D_800B7418;

void func_8005D71C(void) {
    ClearImage(&D_800B7418, 0, 0, 0);
    DrawSync(0);
}
