#include "ac1/ac1.h"

void func_80014774(SVECTOR *ang, SVECTOR *v)
{
    s32 c;

    c = rcos(ang->vx);
    v->vy = -rsin(ang->vx);
    v->vx = (-rsin(ang->vy) * c) >> 12;
    v->vz = (rcos(ang->vy) * c) >> 12;
}
