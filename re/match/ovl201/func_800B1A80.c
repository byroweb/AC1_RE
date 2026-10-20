/* ovl201 0x800B1A80  size 136  variant sweep (tools/match/variants.py): old lever: swap M2C_FIELD(arg0, M2C_UNK  / M2C_FIELD(arg0, u8 *, 0x, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_8005BA2C(M2C_UNK, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern u8 D_80048664;
extern s16 D_801A2318;
extern M2C_UNK func_800B1B08;

void func_800B1A80(void *arg0) {
    func_8005BA2C(0xA8, 0x70, 0x88, 0x48, 7, 0, 0);
    SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    M2C_FIELD(arg0, u8 *, 0x89) = (u8) D_80048664;
    M2C_FIELD(arg0, M2C_UNK **, 0x74) = &func_800B1B08;
}
