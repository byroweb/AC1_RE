/* ovl201 0x800AF470  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800AEBF4();                            /* extern */
extern M2C_UNK func_800AEC84;

void func_800AF470(void *arg0) {
    func_800AEBF4();
    M2C_FIELD(arg0, s8 *, 0x88) = 4;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AEC84;
}
