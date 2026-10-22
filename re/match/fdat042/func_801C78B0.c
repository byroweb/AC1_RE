#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008A798(s32);                         /* extern */
extern u16 D_8019F52A[];
extern s32 D_801AC80C;
extern s32 D_801CD664;

void func_801C78B0(void) {
    s32 temp_a0;

    temp_a0 = D_801CD664;
    if (temp_a0 < 0x1000) {
        D_801CD664 = temp_a0 + 1;
        func_8008A798(temp_a0);
    }
    if (D_8019F52A[0] == 0xFFFE) {
        D_8019F52A[0] = 0;
        D_801AC80C = 1;
    }
}
