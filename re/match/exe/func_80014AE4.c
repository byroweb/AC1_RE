#include "ac1/ac1.h"

void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *ang)
{
    ang->vy = -ratan2(x, z);
    ang->vx = ratan2(y, SquareRoot0(x * x + z * z)) & 0xFFF;
    ang->vz = 0;
}
