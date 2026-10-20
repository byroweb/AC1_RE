/* ovl201 0x80057108  size 136  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern s32 D_801A2574;
extern M2C_UNK func_80052334;

void func_80057108(void *arg0) {
    func_8005BA2C(0x10, 0x28, 0x80, 0x78, 3, 0, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    D_801A2574 = 2;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_80052334;
}
