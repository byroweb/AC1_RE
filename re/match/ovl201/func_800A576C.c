/* ovl201 0x800A576C  size 132  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */

void func_800A576C(void) {
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_800506EC(6);
    obj_find_by_id(5)->kill = 1;
    func_8005C2D0(0x10, 0x28, 0x88, 0x90, 3);
    func_8005C2D0(0xA0, 0x50, 0x98, 0x30, 5);
}
