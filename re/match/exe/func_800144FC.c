#include "ac1/ac1.h"

void func_800144FC(MATRIX *m, SVECTOR *r)
{
    s16 s;
    s32 c;
    s32 x, y;

    s = m->m[0][2];
    c = SquareRoot0(0x1000000 - s * s);
    r->vy = ratan2(s, c);
    if (c == 0) {
        c = 1;
    }
    x = (m->m[2][2] << 12) / c;
    y = (-m->m[1][2] << 12) / c;
    r->vx = ratan2(y, x);
    x = (m->m[0][0] << 12) / c;
    y = (-m->m[0][1] << 12) / c;
    r->vz = ratan2(y, x);
}
