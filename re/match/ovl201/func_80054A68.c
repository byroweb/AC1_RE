/* ovl201 0x80054A68  size 72  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
extern M2C_UNK func_800542F0;

void func_80054A68(void *arg0) {
    func_800506EC(0xA);
    obj_find_by_id(9)->kill = 1;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800542F0;
}
