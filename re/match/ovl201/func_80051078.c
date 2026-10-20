/* ovl201 0x80051078  size 56  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK EnterCriticalSection();                     /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
M2C_UNK free2(s32);                                 /* extern */

void func_80051078(s32 arg0) {
    EnterCriticalSection();
    free2(arg0);
    ExitCriticalSection();
}
