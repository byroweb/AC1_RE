#include "ac1/ac1.h"

s32 func_800126D8(SVECTOR *a, s32 r, s32 r2, SVECTOR *b)
{
    s32 dx, dy, dz;
    s32 d;

    dx = b->vx - a->vx;
    if (dx >= -r && dx <= r) {
        dz = b->vz - a->vz;
        if (dz >= -r && dz <= r) {
            dy = b->vy - a->vy;
            if (dy >= -r && dy <= r) {
                dx >>= 1;
                dz >>= 1;
                dy >>= 1;
                d = dx * dx + dz * dz + dy * dy;
                if (d <= r2) {
                    return d;
                }
            }
        }
    }
    return -1;
}
