/* ovl201 0x800AD350  size 100  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_800AD3B4;

void func_800AD350(void *arg0) {
    func_800506EC(0xE);
    obj_find_by_id(0xD)->kill = 1;
    func_8005C2D0(0x88, 0xA8, 0x88, 0x30, 0xD);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800AD3B4;
}
