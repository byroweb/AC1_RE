/* fdat084 0x801C775C  size 148  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004C318(M2C_UNK);                     /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern u16 D_8019F528;
extern s32 D_801C79FC;

s32 func_801C775C(s32 arg0) {
    if (D_8019F528 == 0) {
        switch (arg0) {                             /* irregular */
        case 0:
            if (D_801C79FC == 0) {
                D_801C79FC = 1;
                func_800529F0(0, 0, 0x28);
            }
            break;
        case 1:
            func_800529F0(0, 0, 0x2B);
            func_800529F0(0, 0, 0x68);
            func_8004C318(0x80);
            break;
        }
    }
    return 1;
}
