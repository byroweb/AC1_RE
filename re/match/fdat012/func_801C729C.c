#include "ac1/ac1.h"

extern u8 D_80031A94[];
extern u8 D_80031B34[];

s32 func_801C729C(s32 arg0, s32 arg1)
{
    if (arg0 == 7 && (D_80031A94[arg1] != 0 || D_80031B34[arg1] != 0)) {
        return 0;
    }
    return 1;
}
