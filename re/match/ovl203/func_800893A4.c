/* ovl203 0x800893A4  size 124  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004DD34(u8);                          /* extern */
extern u8 D_80039C5D;
extern s32 D_8019B48C;
extern u16 D_8019B492;

void func_800893A4(void) {
    if (D_8019B492 == 0x2C) {
        D_8019B48C &= ~0x1400;
        if (D_80039C5D != 0xFF) {
            func_8004DD34(D_80039C5D);
        }
    } else {
        D_8019B48C |= 0x1400;
    }
}
