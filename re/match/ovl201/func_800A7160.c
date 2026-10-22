#include "ac1/ac1.h"

extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern s32 D_8004A278;
extern s32 D_8004A284;
extern s32 D_800BDCCC;
extern u8 D_8017F4B0[8];

s32 func_800A7160(void)
{
    s32 i;
    s32 sum;

    if (TestEvent(D_8004A278) == 1) {
        for (i = 1, sum = 0; i < 8; i++) {
            sum += D_8017F4B0[i];
        }
        if (D_8017F4B0[0] != (u8)sum) {
            D_800BDCCC = 0;
            return 0;
        }
        read(D_8004A284, D_8017F4B0, 8);
        return 1;
    }
    if (D_800BDCCC == 0) {
        func_800309A8(2, 0, 0);
        read(D_8004A284, D_8017F4B0, 8);
        D_800BDCCC = 1;
    }
    return 0;
}
