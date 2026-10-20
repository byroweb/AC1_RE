/* ovl201 0x80051868  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK EnterCriticalSection();                     /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
M2C_UNK func_800309A8(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */

void func_80051868(void) {
    EnterCriticalSection();
    func_800309A8(1, 3, 0x1FA400);
    ExitCriticalSection();
}
