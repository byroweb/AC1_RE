/* ovl201 0x8004FF14  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D8D4(M2C_UNK *, s32, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004AE88;
extern M2C_UNK func_8004FF5C;

void func_8004FF14(void *arg0) {
    func_8005D8D4(&D_8004AE88, arg0 + 0x7C, 4, 0);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004FF5C;
}
