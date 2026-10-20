/* exe 0x80015AC8  size 92  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK EnterCriticalSection();                     /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
M2C_UNK func_800161F8();                            /* extern */

void func_80015AC8(u8 *arg0) {
loop_1:
    EnterCriticalSection();
    if (*arg0 != 0) {
        ExitCriticalSection();
        func_800161F8();
        goto loop_1;
    }
    ExitCriticalSection();
}
