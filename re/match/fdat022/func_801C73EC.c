/* fdat022 0x801C73EC  size 96  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_8019F51C;
extern s32 D_801C7624;
extern M2C_UNK func_801C6758;

void func_801C73EC(void *arg0) {
    if ((M2C_FIELD(arg0, u16 *, 0x160) == 0) && (M2C_FIELD(((M2C_FIELD(M2C_FIELD(arg0, void **, 0), s16 *, 0xA) * 4) + D_8019F51C), s32 *, 0x20) == &func_801C6758)) {
        D_801C7624 += 0x1388;
    }
}
