/* ovl202 0x8005239C  size 32  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

extern s16 D_801ED848;
extern s16 D_801ED84A;
extern s16 D_801ED84C;

void func_8005239C(s16 arg0) {
    D_801ED848 = arg0;
    D_801ED84A = 0;
    D_801ED84C = 0;
}
