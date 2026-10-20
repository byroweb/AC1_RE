/* ovl203 0x80089558  size 76  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800893A4();                            /* extern */
M2C_UNK func_800894B4();                            /* extern */
extern u16 D_8019B490;
extern u16 D_8019B492;

void func_80089558(void) {
    if ((u16) D_8019B492 < 0x2DU) {
        func_800893A4();
    }
    if (D_8019B490 != 0) {
        func_800894B4();
    }
}
