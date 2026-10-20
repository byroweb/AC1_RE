/* ovl201 0x800ACC44  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern M2C_UNK func_800ACCC0;

void func_800ACC44(void *arg0) {
    func_8005BA2C(0x88, 0xA8, 0x88, 0x30, 0xD, 0, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800ACCC0;
}
