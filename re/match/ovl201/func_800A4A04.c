/* ovl201 0x800A4A04  size 112  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK StSetRing(s32, M2C_UNK);                    /* extern */
M2C_UNK StSetStream(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32); /* extern */
M2C_UNK func_800A4E38(s32);                         /* extern */
M2C_UNK func_800B4E84(M2C_UNK);                     /* extern */
M2C_UNK func_800B5128(M2C_UNK);                     /* extern */
extern s32 D_8017F434;

void func_800A4A04(s32 arg0, M2C_UNK arg1) {
    func_800B4E84(0);
    func_800B5128(arg1);
    StSetRing(D_8017F434, 0x10);
    StSetStream(1, 1, -1, 0, 0);
    func_800A4E38(arg0);
}
