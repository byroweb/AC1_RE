/* exe 0x800161F8  size 40  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK VSync(M2C_UNK);                             /* extern */

void func_800161F8(void) {
    DrawSync(0);
    VSync(0);
}
