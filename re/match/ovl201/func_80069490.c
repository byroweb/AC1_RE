/* ovl201 0x80069490  size 152  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8006BA08();                            /* extern */
extern M2C_UNK func_80069528;

void func_80069490(void *arg0) {
    func_8006BA08();
    func_800506EC(6);
    obj_find_by_id(5)->kill = 1;
    func_800506EC(8);
    obj_find_by_id(7)->kill = 1;
    func_800506EC(0xA);
    obj_find_by_id(9)->kill = 1;
    func_8005C2D0(0x50, 0x32, 0x68, 0x90, 9);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80069528;
}
