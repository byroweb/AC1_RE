#include "ac1/ac1.h"
/* ovl201 0x80073740 - func_80050AF4 for every equipped part category (ids
 * offset into one table), the two weapon slots 6 / 7 in ascending order. */

extern u8 D_80031A58[];
extern void func_80050AF4(s32 kind, s16 id);

void func_80073740(void)
{
    if (D_80031A58[0] != 255) {
        func_80050AF4(4, D_80031A58[0] + 3);
    }
    if (D_80031A58[11] != 255) {
        func_80050AF4(6, D_80031A58[11] + 6);
    }
    if (D_80031A58[3] != 255) {
        func_80050AF4(5, D_80031A58[3] + 38);
    }
    if (D_80031A58[2] != 255) {
        func_80050AF4(3, D_80031A58[2] + 54);
    }
    if (D_80031A58[1] != 255) {
        func_80050AF4(14, D_80031A58[1] + 64);
    }
    if (D_80031A58[6] != 255 && D_80031A58[7] != 255) {
        if (D_80031A58[6] < D_80031A58[7]) {
            func_80050AF4(9, D_80031A58[6] + 70);
            func_80050AF4(10, D_80031A58[7] + 70);
        } else {
            func_80050AF4(10, D_80031A58[7] + 70);
            func_80050AF4(9, D_80031A58[6] + 70);
        }
    } else {
        if (D_80031A58[6] != 255) {
            func_80050AF4(9, D_80031A58[6] + 70);
        }
        if (D_80031A58[7] != 255) {
            func_80050AF4(10, D_80031A58[7] + 70);
        }
    }
    if (D_80031A58[4] != 255) {
        func_80050AF4(7, D_80031A58[4] + 104);
    }
    if (D_80031A58[5] != 255) {
        func_80050AF4(8, D_80031A58[5] + 108);
    }
    if (D_80031A58[9] != 255) {
        func_80050AF4(12, D_80031A58[9] + 124);
    }
    if (D_80031A58[8] != 255) {
        func_80050AF4(11, D_80031A58[8] + 143);
    }
}
