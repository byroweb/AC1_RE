#include "ac1/ac1.h"

extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern s32 D_8004A288;
extern u8 D_8017F4B8[8];

s32 func_800A70EC(void)
{
    s32 i;
    s32 sum;

    if (func_800309A8(3, 1, 0) == 0) {
        return 0;
    }
    for (i = 1, sum = 0; i < 8; i++) {
        sum += i;
    }
    D_8017F4B8[0] = sum;
    write(D_8004A288, D_8017F4B8, 8);
    return 1;
}
