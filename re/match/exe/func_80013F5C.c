#include "ac1/ac1.h"

void func_80013F5C(SVECTOR *a, SVECTOR *b, MATRIX *rx, MATRIX *ry, MATRIX *out)
{
    s32 dx, dy, dz;
    s32 h2;
    s32 h, l;
    s32 sn, cs, sy, cy;
    s16 t;

    dx = b->vx - a->vx;
    dz = b->vz - a->vz;
    h2 = dx * dx + dz * dz;
    dy = b->vy - a->vy;
    h = SquareRoot0(h2);
    if (h == 0) {
        return;
    }
    l = SquareRoot0(h2 + dy * dy);
    if (l == 0) {
        return;
    }
    sn = (dy << 12) / l;
    cs = (h << 12) / l;
    sy = (dz << 12) / h;
    cy = (dx << 12) / h;
    rx->m[0][0] = 0x1000;
    rx->m[0][1] = 0;
    rx->m[0][2] = 0;
    rx->m[1][0] = 0;
    rx->m[2][0] = 0;
    t = cs;
    rx->m[1][1] = t;
    rx->m[2][2] = t;
    rx->m[1][2] = -sn;
    rx->m[2][1] = sn;
    t = sy;
    ry->m[0][0] = t;
    ry->m[0][1] = 0;
    ry->m[0][2] = -cy;
    ry->m[1][0] = 0;
    ry->m[1][1] = 0x1000;
    ry->m[1][2] = 0;
    ry->m[2][0] = cy;
    ry->m[2][1] = 0;
    ry->m[2][2] = t;
    MulMatrix0(rx, ry, out);
}
