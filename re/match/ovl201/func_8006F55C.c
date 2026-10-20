/* ovl201 0x8006F55C  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK MoveImage(M2C_UNK *, M2C_UNK, M2C_UNK);     /* extern */
extern M2C_UNK D_800B8134;

void func_8006F55C(void) {
    MoveImage(&D_800B8134, 0x39A, 0x40);
    DrawSync(0);
}
