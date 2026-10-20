/* ovl201 0x8004FFF4  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_80050804(M2C_UNK, M2C_UNK);            /* extern */
M2C_UNK func_8005B2CC();                            /* extern */
M2C_UNK func_800A2D0C(M2C_UNK);                     /* extern */
M2C_UNK func_800A448C(M2C_UNK, M2C_UNK);            /* extern */
extern s8 D_80039CA9;

void func_8004FFF4(void *arg0) {
    func_800A448C(5, 0);
    func_800A448C(8, 0);
    func_80050804(4, 0);
    func_800506EC(2);
    obj_find_by_id(1)->kill = 1;
    D_80039CA9 = 0;
    func_8005B2CC();
    func_800A2D0C(0);
    M2C_FIELD(arg0, s8 *, 1) = 1;
}
