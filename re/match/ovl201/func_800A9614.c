/* ovl201 0x800A9614  size 212  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005DD38(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
M2C_UNK func_8005DE08(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern s16 D_801A2318;

void func_800A9614(void) {
    func_8005DD38(0x3C0, 0x80, 0xF);
    func_8005DE08(0x3C0, 0x98, 0xF);
    func_8005BA2C(0x10, 0x28, 0x88, 0x60, 3, 0, 0);
    func_8005BA2C(0xA0, 0x10, 0x98, 0x38, 5, 0xA, 0);
    func_8005BA2C(0xB8, 0x60, 0x6C, 0x68, 7, 1, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
}
