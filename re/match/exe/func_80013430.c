#include "ac1/ac1.h"

extern s32 func_80014988(s32 x, s32 y, s32 z);

s32 func_80013430(VECTOR *a, VECTOR *b, s32 len, SVECTOR *out)
{
    s32 dx, dy, dz;
    s32 d;

    dx = b->vx - a->vx;
    dy = b->vy - a->vy;
    dz = b->vz - a->vz;
    d = func_80014988(dx, dy, dz);
    out->vx = dx * len / d;
    out->vy = dy * len / d;
    out->vz = dz * len / d;
    return d;
}
