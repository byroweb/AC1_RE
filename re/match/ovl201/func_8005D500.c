/* ovl201 0x8005D500  size 68  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

s32 SsVabTransCompleted(M2C_UNK);                   /* extern */
extern M2C_UNK func_8005D544;

void func_8005D500(void *arg0) {
    if ((SsVabTransCompleted(0) << 0x10) != 0) {
        M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8005D544;
    }
}
