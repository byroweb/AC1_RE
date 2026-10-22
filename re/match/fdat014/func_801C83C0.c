#include "ac1/ac1.h"

extern s32 func_801C830C(s32 n);
extern s32 D_801C8540;
extern s32 D_801C8544;

void func_801C83C0(s32 *a, s32 *b)
{
    s32 t;
    s32 u;

    *a += D_801C8540;
    t = func_801C830C(21) + func_801C830C(22);
    u = D_801C8544 - t;
    D_801C8544 = u;
    *b -= u * 1000;
}
