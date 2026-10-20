/* ovl201 0x8007092C  size 168  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005D71C();                            /* extern */
M2C_UNK func_80070834();                            /* extern */
extern s16 D_801A2318;
extern s32 D_801A2574;

void func_8007092C(void) {
    func_8005D71C();
    func_80070834();
    func_8005BA2C(0x20, 0x28, 0x80, 0xA8, 3, 0, 0);
    func_8005BA2C(0xB0, 0x18, 0x80, 0x90, 5, 1, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    D_801A2574 = 2;
}
