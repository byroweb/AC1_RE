/* ovl201 0x8007C6FC  size 220  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D15C(M2C_UNK *, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8007FFEC(s32);                         /* extern */
extern M2C_UNK D_8004C498;
extern M2C_UNK D_8004C4A8;
extern M2C_UNK D_800B8454;
extern M2C_UNK D_800B8460;

void func_8007C6FC(s32 arg0) {
    func_8005DD38(0x3C0, 0x10, 0xF);
    func_8005DE08(0x3C0, 0x28, 0xF);
    func_8005D15C(&D_8004C498, 0x3C2, 0x44);
    func_8005D15C(&D_8004C4A8, 0x3C2, 0x5C);
    func_8005D754(3, &D_800B8454, 0, 0, 4);
    func_8005D754(3, &D_800B8454, 0, 0x18, 4);
    func_8005D754(2, &D_800B8460, 0, 0, 4);
    func_8007FFEC(arg0);
}
