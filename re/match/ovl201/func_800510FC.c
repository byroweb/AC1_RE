/* ovl201 0x800510FC  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK EnterCriticalSection();                     /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
M2C_UNK InitHeap2(M2C_UNK *, M2C_UNK);              /* extern */
extern M2C_UNK D_800CF37C;

void func_800510FC(void) {
    EnterCriticalSection();
    InitHeap2(&D_800CF37C, 0xAF000);
    ExitCriticalSection();
}
