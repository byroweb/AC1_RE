/* ovl201 0x8008B698  size 212  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
extern s16 D_801A2318;
extern s32 D_801A2568;
extern s32 D_801A256C;

void func_8008B698(void *arg0) {
    if ((D_801A2568 & 0x8000) && !(D_801A256C & 0x8000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, s32 *, 0x48) = 4;
    }
    if ((D_801A2568 & 0x2000) && !(D_801A256C & 0x2000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        M2C_FIELD(arg0, s32 *, 0x48) = 0x49;
    }
}
