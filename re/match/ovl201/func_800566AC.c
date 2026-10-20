/* ovl201 0x800566AC  size 196  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_800549D0;

void func_800566AC(void *arg0) {
    SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
    func_800506EC(4);
    func_800506EC(6);
    obj_find_by_id(3)->kill = 1;
    obj_find_by_id(5)->kill = 1;
    func_8005C2D0(0x10, 0x28, 0x78, 0x18, 3);
    func_8005C2D0(0x10, 0x40, 0x120, 0x94, 5);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800549D0;
}
