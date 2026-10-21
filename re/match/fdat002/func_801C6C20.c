/* fdat002 0x801C6C20  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_8019F51C;
extern s32 D_801C6DF0;
extern s32 D_801C6DF4;
extern M2C_UNK func_801C5E60;
extern M2C_UNK func_801C5E84;

void func_801C6C20(void *arg0) {
    void *temp_a0;

    if (M2C_FIELD(arg0, u16 *, 0x160) == 0) {
        temp_a0 = M2C_FIELD(arg0, void **, 0);
        if (M2C_FIELD(((M2C_FIELD(temp_a0, s16 *, 0xA) * 4) + D_8019F51C), s32 *, 0x20) == &func_801C5E60) {
            D_801C6DF4 += 0x1F4;
        }
        if (M2C_FIELD(((M2C_FIELD(temp_a0, s16 *, 0xA) * 4) + D_8019F51C), s32 *, 0x20) == &func_801C5E84) {
            D_801C6DF0 -= 0x3E8;
        }
    }
}
