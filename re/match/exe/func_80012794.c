#include "ac1_types.h"
/* exe 0x80012794  size 740: cylinder-vs-cylinder overlap test; when moving
 * (vel != 0) it either snaps pos onto the other cylinder's top/bottom or
 * pushes it out sideways. Returns 1 on contact, 0 otherwise. */
typedef struct {
    s16 vx, vy, vz, pad;
} SVECTOR;

extern long SquareRoot0(long a);

int func_80012794(SVECTOR *pos, int r1, int h1, SVECTOR *vel, SVECTOR *opos, int r2, int h2)
{
    int r;
    int dx;
    int dz;
    int d2;
    int rr;
    int dy;
    int ady;
    int avy;
    int a;
    int b;
    int len;
    s16 t;

    r = r1 + r2;
    dx = opos->vx - pos->vx;
    if (dx < -r || r < dx) {
        goto fail;
    }
    dz = opos->vz - pos->vz;
    if (dz < -r || r < dz) {
        goto fail;
    }
    if (opos->vy > pos->vy) {
        if (opos->vy - h2 <= pos->vy) {
            goto yok;
        }
        goto fail;
    }
    if (pos->vy - h1 > opos->vy) {
        goto fail;
    }
yok:
    d2 = dx * dx + dz * dz;
    rr = r * r;
    if (rr < d2) {
        goto fail;
    }
    if (vel == 0) {
ok:
        return 1;
    }
    if (vel->vy <= 0) {
        goto other;
    }
    dy = pos->vy - (opos->vy - h2) + 1;
common:
    ady = abs(dy);
    avy = abs(vel->vy);
    a = abs(vel->vx) * ady / avy;
    b = abs(vel->vz) * ady / avy;
    a = (a + abs(dx)) >> 2;
    b = (b + abs(dz)) >> 2;
    if (rr >> 4 >= a * a + b * b) {
        pos->vy -= dy;
        vel->vy -= dy;
        goto ok;
    }
push:
    len = SquareRoot0(d2);
    if (len == 0) {
        goto fail;
    }
    r += r >> 2;
    dx = dx * r / len;
    dz = dz * r / len;
    t = pos->vx - vel->vx;
    pos->vx = opos->vx - dx;
    vel->vx = pos->vx - t;
    t = pos->vz - vel->vz;
    pos->vz = opos->vz - dz;
    vel->vz = pos->vz - t;
    goto ok;
other:
    if (vel->vy >= 0) {
        goto push;
    }
    dy = pos->vy - h1 - opos->vy - 1;
    goto common;
fail:
    return 0;
}
