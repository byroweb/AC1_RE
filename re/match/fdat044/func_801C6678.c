/* fdat044 0x801C6678  size 160  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004C318(M2C_UNK);                     /* extern */
M2C_UNK func_800529F0(M2C_UNK, M2C_UNK, M2C_UNK);   /* extern */
extern u16 D_8019F528;
extern u16 D_8019F52C;
extern s32 D_801CCA40;

s32 func_801C6678(s32 arg0) {
    if (D_8019F528 == 0) {
        switch (arg0) {                             /* irregular */
        case 0:
            if (D_801CCA40 < 2) {
                func_800529F0(0, 0, 0x31);
                D_8019F52C += (u16) D_8019F52C >> 1;
            }
            break;
        case 1:
            func_800529F0(0, 0, 0x68);
            D_8019F52C = 0;
            func_8004C318(0x80);
            break;
        }
    }
    return 1;
}
