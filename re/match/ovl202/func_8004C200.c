/* ovl202 0x8004C200  size 52  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-19 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_800309A8(M2C_UNK, M2C_UNK, M2C_UNK *); /* extern */
extern s8 D_8008D920;
extern M2C_UNK func_8004C1BC;

void func_8004C200(void) {
    D_8008D920 = 0;
    func_800309A8(4, 0, &func_8004C1BC);
}
