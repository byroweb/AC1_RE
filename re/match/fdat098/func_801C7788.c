/* fdat098 0x801C7788  size 140  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern s32 D_80049E74;
extern s32 D_8019F51C;
extern s32 D_801CE3B4;
extern M2C_UNK func_801C5678;

void func_801C7788(void *arg0) {
    s32 temp_v0;

    if ((M2C_FIELD(arg0, u16 *, 0x160) == 0) && (M2C_FIELD(((M2C_FIELD(M2C_FIELD(arg0, void **, 0), s16 *, 0xA) * 4) + D_8019F51C), s32 *, 0x20) == &func_801C5678)) {
        temp_v0 = D_801CE3B4 + 1;
        D_801CE3B4 = temp_v0;
        if (temp_v0 == 2) {
            D_80049E74 = 0;
            func_800529F0(0, 0, 0x5C);
        }
    }
}
