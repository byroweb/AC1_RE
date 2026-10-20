/* ovl201 0x800B2584  size 324  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
void *func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
s32 func_8005D80C(M2C_UNK, M2C_UNK *, M2C_UNK, M2C_UNK, s32, s32); /* extern */
extern M2C_UNK D_8004DB68;
extern M2C_UNK D_8004DB70;
extern M2C_UNK D_8004DB78;
extern M2C_UNK D_800BDEC0;
extern M2C_UNK D_800BDECC;
extern s16 D_801A2318;

void func_800B2584(void *arg0) {
    void *temp_v0;

    M2C_FIELD(arg0, s32 *, 0x7C) = func_8005D80C(1, &D_8004DB68, 0x10, 0x24, 0x40, 2);
    M2C_FIELD(arg0, s32 *, 0x80) = func_8005D80C(1, &D_8004DB70, 0x10, 0x4C, 0x40, 2);
    M2C_FIELD(arg0, s32 *, 0x84) = func_8005D80C(1, &D_8004DB78, 0x10, 0xE4, 0x40, 2);
    temp_v0 = func_8005CAE0(&D_800BDEC0, 0x118, 0x56, 2);
    M2C_FIELD(arg0, void **, 0x88) = temp_v0;
    M2C_FIELD(temp_v0, s8 *, 2) = 0;
    M2C_FIELD(arg0, void **, 0x8C) = func_8005CAE0(&D_800BDECC, 0x118, 0xCC, 2);
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x88), s16 *, 0x16) = 0xB;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0x8C), s16 *, 0x16) = 0xB;
    func_8005BA2C(0x1C, 0x50, 0x100, 0x90, 3, 0, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
}
