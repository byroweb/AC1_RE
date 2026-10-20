/* ovl203 0x8004BF6C  size 40  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s32 D_8019B48C;
extern u16 D_8019B490;

void func_8004BF6C(s32 arg0) {
    D_8019B48C = arg0;
    if (D_8019B490 == 0) {
        D_8019B490 = 0x5A;
    }
}
