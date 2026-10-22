#include "ac1/ac1.h"

extern void func_80073714(s32, s32);

s32 func_801C5F3C(s32 arg0, s32 arg1, ...)
{
    func_80073714(arg0, 0x7FFFFFFF);
    return 0;
}
