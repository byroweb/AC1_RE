#include "ac1/ac1.h"

s16 func_8008CFD4(s8 idx, s8 add)
{
    u16 tbl[10] = { 0, 10, 13, 29, 61, 68, 76, 87, 93, 127 };

    return tbl[idx] + add;
}
