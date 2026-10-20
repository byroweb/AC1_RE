/* ovl201 0x80053924  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80053988;

void func_80053924(void *arg0) {
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_8005C2D0(0x60, 0x60, 0x80, 0x30, 3);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80053988;
}
