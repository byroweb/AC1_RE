#include "ac1_types.h"
extern u8 D_80090920;
extern u8 D_80090921;
extern u8 D_80090922;
void func_8008ABD4(u8 v)
{
    D_80090922 = v;
    D_80090921 = v;
    D_80090920 = v;
}
