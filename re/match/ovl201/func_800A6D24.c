/* ovl201 0x800A6D24  size 128  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern M2C_UNK func_80072838;
extern M2C_UNK func_800A6DA4;

void func_800A6D24(void *arg0) {
    func_800506EC(6);
    obj_find_by_id(5)->kill = 1;
    func_8005C2D0(0xB0, 0x18, 0x80, 0x90, 5);
    obj_find_by_handle(&func_80072838)->kill = 1;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800A6DA4;
}
