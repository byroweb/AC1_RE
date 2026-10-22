#include "ac1/ac1.h"

extern s32 D_801A5F30[];
void func_800574D8(s32);

void func_80057994(u16 i, s32 v)
{
    D_801A5F30[0] = D_801A5F30[i - 1] = v;
    func_800574D8(v);
}
