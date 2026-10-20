/* ovl201 0x80052F68  size 160  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80053008;

void func_80052F68(void *arg0) {
    func_800506EC(4);
    func_800506EC(6);
    obj_find_by_id(3)->kill = 1;
    obj_find_by_id(5)->kill = 1;
    M2C_FIELD(arg0, s8 *, 0xA0) = 0;
    func_8005C2D0(0x10, 0x28, 0x80, 0x78, 3);
    func_8005C2D0(0xA0, 0x20, 0x90, 0xA0, 5);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80053008;
}
