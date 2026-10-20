/* ovl201 0x80090BFC  size 60  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80090668();                            /* extern */
M2C_UNK func_800908F8();                            /* extern */
extern M2C_UNK func_80090C38;

void func_80090BFC(void *arg0) {
    func_80090668();
    func_800908F8();
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80090C38;
}
