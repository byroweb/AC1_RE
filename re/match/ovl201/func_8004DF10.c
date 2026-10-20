/* ovl201 0x8004DF10  size 308  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
s32 func_8004F2CC(M2C_UNK, M2C_UNK, s8);            /* extern */
void *func_8005CAE0(M2C_UNK *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
M2C_UNK func_8005CC48();                            /* extern */
extern M2C_UNK D_800B91C4;
extern M2C_UNK D_800B91D0;
extern s16 D_801A2318;
extern M2C_UNK func_8004E044;

void func_8004DF10(void *arg0) {
    void *temp_v0;
    void *temp_v0_2;

    SsUtKeyOn(D_801A2318, 0, 8, 0x3C, 0, 0x7F, 0x7F);
    M2C_FIELD(arg0, s32 *, 0x80) = func_8004F2CC(2, 1, M2C_FIELD(arg0, s8 *, 0xAE));
    M2C_FIELD(arg0, s32 *, 0x84) = func_8004F2CC(2, 2, M2C_FIELD(arg0, s8 *, 0xAE));
    M2C_FIELD(arg0, s32 *, 0x88) = func_8004F2CC(2, 3, M2C_FIELD(arg0, s8 *, 0xAE));
    M2C_FIELD(arg0, s32 *, 0x8C) = func_8004F2CC(2, 4, M2C_FIELD(arg0, s8 *, 0xAE));
    M2C_FIELD(arg0, s32 *, 0x90) = func_8004F2CC(2, 5, M2C_FIELD(arg0, s8 *, 0xAE));
    M2C_FIELD(arg0, s32 *, 0x94) = func_8004F2CC(2, 6, M2C_FIELD(arg0, s8 *, 0xAE));
    func_8005CC48();
    temp_v0 = func_8005CAE0(&D_800B91D0, 0xD2, 0x84, 2);
    M2C_FIELD(arg0, void **, 0xA0) = temp_v0;
    M2C_FIELD(temp_v0, s16 *, 0x16) = 0xB;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xA0), s8 *, 2) = 0;
    temp_v0_2 = func_8005CAE0(&D_800B91C4, 0x5F, 0x84, 2);
    M2C_FIELD(arg0, void **, 0xA4) = temp_v0_2;
    M2C_FIELD(temp_v0_2, s16 *, 0x16) = 0xB;
    M2C_FIELD(M2C_FIELD(arg0, void **, 0xA4), s8 *, 2) = 0;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_8004E044;
}
