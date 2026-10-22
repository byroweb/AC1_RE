#include "ac1/ac1.h"

void func_80013CC0(SVECTOR *v, MATRIX *m)
{
    MATRIX rz;
    s32 x, y, z;
    s32 h2;
    s32 h, l;
    s32 sn, cs, sz, cz;
    s16 t;

    x = v->vx;
    y = v->vy;
    h2 = x * x + y * y;
    z = v->vz;
    h = SquareRoot0(h2);
    if (h == 0) {
        return;
    }
    l = SquareRoot0(h2 + z * z);
    if (l == 0) {
        return;
    }
    sn = (z << 12) / l;
    cs = (h << 12) / l;
    sz = (y << 12) / h;
    cz = (x << 12) / h;
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
    t = sz;
    rz.m[0][0] = t;
    rz.m[0][1] = cz;
    rz.m[0][2] = 0;
    rz.m[1][0] = -cz;
    rz.m[1][1] = t;
    rz.m[1][2] = 0;
    rz.m[2][0] = 0;
    rz.m[2][1] = 0;
    rz.m[2][2] = 0x1000;
    MulMatrix(m, &rz);
}
