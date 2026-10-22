#include "ac1/ac1.h"

extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *ang);

s32 func_80015258(SVECTOR *from, SVECTOR *ref, s32 speed, SVECTOR *pos, SVECTOR *vel, SVECTOR *out, SVECTOR *ang, s32 n)
{
    s32 dx, dy, dz;
    s32 t;
    s32 a;

    *out = *pos;
    for (;;) {
        dx = out->vx - from->vx;
        dy = out->vy - from->vy;
        dz = out->vz - from->vz;
        t = SquareRoot0(dx * dx + dy * dy + dz * dz) / speed;
        if (--n == -1) {
            break;
        }
        out->vx = pos->vx + vel->vx * t;
        out->vy = pos->vy + vel->vy * t;
        out->vz = pos->vz + vel->vz * t;
    }
    if (ang != 0) {
        func_80014AE4(dx, dy, dz, ang);
        if (ang->vx >= 0x800) {
            ang->vx -= 0x1000;
        }
        a = (u16)(ang->vy - ref->vy) & 0xFFF;
        ang->vy = a;
        if (a >= 0x800) {
            ang->vy = a - 0x1000;
        }
    }
    return t;
}
