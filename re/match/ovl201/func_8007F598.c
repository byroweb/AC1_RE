/* ovl201 0x8007F598  size 296  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D2DC(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004C4B4;
extern M2C_UNK D_8004C63C;
extern M2C_UNK func_8007F6C0;

void func_8007F598(void *arg0) {
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x8C), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x90), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x94), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x98), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x9C), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xA0), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xA4), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xA8), s8 *, 1) = 1;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xAC), s8 *, 1) = 1;
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_800506EC(8);
    obj_find_by_id(7)->kill = 1;
    func_8005C2D0(0x10, 0x28, 0x90, 0x80, 3);
    func_8005C2D0(0x14, 0xB1, 0x84, 0x18, 7);
    func_8005D15C(&D_8004C4B4, 0x3C2, 0x74);
    func_8005D2DC(&D_8004C63C, 0x3C0, 0x84);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8007F6C0;
}
