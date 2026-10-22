/* fdat048 0x801C5EF0  size 80  sweep.py best draft: ctx, closeness 0.800 aligned / 0.700 positional */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008A80C();                            /* extern */
extern u16 D_801987D8[];
extern s32 D_8019F524;

void func_801C5EF0(void) {
    func_8008A80C();
    if (D_8019F524 & 0x100) {
        D_801987D8[0] |= 1;
    }
}
