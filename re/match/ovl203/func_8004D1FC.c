#include "ac1/ac1.h"

extern s32 D_80039D18;
extern s32 D_80039D30;
extern void func_8004D394(s32 a, s32 b, s32 c, s32 d, s32 e);

void func_8004D1FC(s32 a, s32 b)
{
    if (D_80039D18 == D_80039D30) {
        func_8004D394(a, 0, b, b, 0);
    }
}
