/* ovl201 0x8008651C  size 312  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
extern M2C_UNK D_8004CB60;
extern M2C_UNK D_8004CB6C;
extern M2C_UNK D_8004CB7C;
extern M2C_UNK D_8004CB88;
extern M2C_UNK D_8004CB98;
extern M2C_UNK D_8004CBA4;
extern M2C_UNK D_8004CBB4;
extern M2C_UNK D_800B83C4;

void func_8008651C(void) {
    func_8005D754(3, &D_800B83C4, 0, 0, 4);
    func_8005D80C(2, &D_8004CB60, 0x10, 8, 4, 4);
    func_8005D80C(2, &D_8004CB6C, 0x10, 8, 0x1C, 4);
    func_8005D80C(2, &D_8004CB7C, 0x10, 8, 0x34, 4);
    func_8005D80C(2, &D_8004CB88, 0x10, 8, 0x4C, 4);
    func_8005D80C(2, &D_8004CB98, 0x10, 8, 0x64, 4);
    func_8005D80C(2, &D_8004CBA4, 0x10, 8, 0x7C, 4);
    func_8005D80C(2, &D_8004CBB4, 0x10, 8, 0x94, 4);
}
