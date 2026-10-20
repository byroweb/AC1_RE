/* ovl201 0x8007D840  size 244  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_8007FED0(s32);                         /* extern */
extern M2C_UNK D_800B8454;
extern M2C_UNK D_800B8478;

void func_8007D840(s32 arg0) {
    func_8005D754(4, &D_800B8454, 0, 0, 4);
    func_8005D754(4, &D_800B8454, 0, 0x18, 4);
    func_8005D754(4, &D_800B8454, 0, 0x30, 4);
    func_8005D754(4, &D_800B8454, 0, 0x48, 4);
    func_8005D754(4, &D_800B8454, 0, 0x60, 4);
    func_8005D754(4, &D_800B8454, 0, 0x78, 4);
    func_8005D754(3, &D_800B8478, 0, 4, 4);
    func_8007FED0(arg0);
}
