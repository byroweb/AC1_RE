/* ovl201 0x800A2FE0  size 108  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800A448C(u8, M2C_UNK);                 /* extern */
extern u8 D_800BDBBC;
extern M2C_UNK func_800A304C;

void func_800A2FE0(void *arg0) {
    u8 temp_v0;

    func_800A448C(D_800BDBBC, 1);
    temp_v0 = D_800BDBBC + 1;
    D_800BDBBC = temp_v0;
    if ((temp_v0 & 0xFF) == 2) {
        D_800BDBBC = 0;
    }
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A304C;
}
