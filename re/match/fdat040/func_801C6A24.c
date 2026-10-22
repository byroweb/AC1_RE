/* fdat040 0x801C6A24  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_8019F51C;
extern s32 D_801CCB90;
extern M2C_UNK func_801C52C0;

void func_801C6A24(void *arg0) {
    if ((M2C_FIELD(arg0, u16 *, 0x160) == 0) && (M2C_FIELD(((M2C_FIELD(M2C_FIELD(arg0, void **, 0), s16 *, 0xA) * 4) + D_8019F51C), s32 *, 0x20) == &func_801C52C0)) {
        D_801CCB90 += 0xBB8;
    }
}
