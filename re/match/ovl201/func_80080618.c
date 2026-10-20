/* ovl201 0x80080618  size 436  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005D754(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_80081010(s32);                         /* extern */
extern M2C_UNK D_800B8454;
extern M2C_UNK D_800B84FC;
extern M2C_UNK D_800B8508;
extern M2C_UNK D_800B8514;
extern M2C_UNK D_800B8520;
extern M2C_UNK D_800B852C;
extern M2C_UNK D_800B8538;
extern M2C_UNK D_800B8544;

void func_80080618(s32 arg0) {
    func_8005D754(3, &D_800B8454, 0, 0, 4);
    func_8005D754(3, &D_800B8454, 0, 0x18, 4);
    func_8005D754(3, &D_800B8454, 0, 0x30, 4);
    func_8005D754(3, &D_800B8454, 0, 0x48, 4);
    func_8005D754(3, &D_800B8454, 0, 0x60, 4);
    func_8005D754(3, &D_800B8454, 0, 0x78, 4);
    func_8005D754(3, &D_800B8454, 0, 0x90, 4);
    func_8005D754(2, &D_800B84FC, 8, 4, 4);
    func_8005D754(2, &D_800B8508, 8, 0x1C, 4);
    func_8005D754(2, &D_800B8514, 8, 0x34, 4);
    func_8005D754(2, &D_800B8520, 8, 0x4C, 4);
    func_8005D754(2, &D_800B852C, 8, 0x64, 4);
    func_8005D754(2, &D_800B8538, 8, 0x7C, 4);
    func_8005D754(2, &D_800B8544, 8, 0x94, 4);
    func_80081010(arg0);
}
