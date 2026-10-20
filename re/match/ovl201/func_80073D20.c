/* ovl201 0x80073D20  size 188  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_80051450(M2C_UNK, u8);                 /* extern */
extern u8 D_80031A5C;
extern u8 D_80031A5E;
extern u8 D_80031A5F;
extern u8 D_80031A60;
extern u8 D_80031A61;

void func_80073D20(void) {
    if ((D_80031A5E != 0xFF) && ((u8) D_80031A5E >= 0x1FU)) {
        func_80051450(0xC, D_80031A5E);
    }
    if ((D_80031A5F != 0xFF) && ((u8) D_80031A5F >= 0x1FU)) {
        func_80051450(0xD, D_80031A5F);
    }
    if (D_80031A5C != 0xFF) {
        func_80051450(0xE, D_80031A5C);
    }
    if (D_80031A60 != 0xFF) {
        func_80051450(0x12, D_80031A60);
    }
    if (D_80031A61 != 0xFF) {
        func_80051450(0x10, D_80031A61);
    }
}
