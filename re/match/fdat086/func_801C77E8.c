/* fdat086 0x801C77E8  size 156  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004C318(M2C_UNK);                     /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern u16 D_8019F528;
extern s32 D_801CD538;

void func_801C77E8(s32 arg0) {
    if (D_8019F528 == 0) {
        switch (arg0) {                             /* irregular */
        case 0:
            func_800529F0(0, 0, 0x9C);
            func_800529F0(0, 0, 0x68);
            func_8004C318(0x80);
            return;
        case 1:
            func_800529F0(0, 0, 0x9D);
            func_800529F0(0, 0, 0x95);
            func_8004C318(0x100);
            D_801CD538 = -0x7530;
            break;
        }
    }
}
