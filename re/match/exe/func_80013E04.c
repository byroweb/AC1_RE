#include "ac1/ac1.h"

void func_80013E04(SVECTOR *a, SVECTOR *b, MATRIX *m)
{
    MATRIX ry;
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
    m->m[0][0] = 0x1000;
    m->m[0][1] = 0;
    m->m[0][2] = 0;
    m->m[1][0] = 0;
    m->m[2][0] = 0;
    t = cs;
    m->m[1][1] = t;
    m->m[2][2] = t;
    m->m[1][2] = -sn;
    m->m[2][1] = sn;
    t = sy;
    ry.m[0][0] = t;
    ry.m[0][1] = 0;
    ry.m[0][2] = -cy;
    ry.m[1][0] = 0;
    ry.m[1][1] = 0x1000;
    ry.m[1][2] = 0;
    ry.m[2][0] = cy;
    ry.m[2][1] = 0;
    ry.m[2][2] = t;
    MulMatrix(m, &ry);
}
