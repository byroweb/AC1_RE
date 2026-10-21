#include "ac1/ac1.h"

extern s32 D_800BDCD0;
extern u8 D_800BDCD4;
extern s32 func_800162A0(s32 a);

s32 func_800A4EAC(void)
{
    if (func_800162A0(D_800BDCD0) >= 7866) {
        D_800BDCD4 = 1;
        return 0;
    }
    return 1;
}
