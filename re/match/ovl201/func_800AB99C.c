/* ovl201 0x800AB99C  size 372  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
M2C_UNK func_800ADB40(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_8004D464;
extern M2C_UNK D_8004D470;
extern M2C_UNK D_8004D480;
extern M2C_UNK D_8004D490;
extern M2C_UNK D_8004D49C;
extern M2C_UNK D_800BDD34;

void func_800AB99C(s32 arg0) {
    func_800ADB40(arg0 + 0x7D, 0, 0, 0xA);
    func_8005D754(3, &D_800BDD34, 0, 0, 0xA);
    func_8005D754(3, &D_800BDD34, 0, 0x18, 0xA);
    func_8005D754(3, &D_800BDD34, 0, 0x30, 0xA);
    func_8005D754(3, &D_800BDD34, 0, 0x48, 0xA);
    func_8005D754(3, &D_800BDD34, 0, 0x60, 0xA);
    func_8005D80C(2, &D_8004D464, 0x10, 8, 4, 0xA);
    func_8005D80C(2, &D_8004D470, 0x10, 8, 0x1C, 0xA);
    func_8005D80C(2, &D_8004D480, 0x10, 8, 0x34, 0xA);
    func_8005D80C(2, &D_8004D490, 0x10, 8, 0x4C, 0xA);
    func_8005D80C(2, &D_8004D49C, 0x10, 8, 0x64, 0xA);
}
