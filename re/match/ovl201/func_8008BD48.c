/* ovl201 0x8008BD48  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK DrawSync(M2C_UNK);                          /* extern */
M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004CC08;
extern M2C_UNK func_8008BD94;

void func_8008BD48(void *arg0) {
    func_8005D15C(&D_8004CC08, 0x380, 0);
    DrawSync(0);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8008BD94;
}
